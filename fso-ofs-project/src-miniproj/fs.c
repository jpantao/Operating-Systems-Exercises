#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>

#include "disk.h"
#include "fs.h"

#define MIN(X, Y) ((X) > (Y) ? (Y) : (X))

/*******
 * FSO OFS layout (there is no bootBlock)
 * FS block size = disk block size (1KB)
 * block#	| content
 * 0		| super block (with list of dir blocks)
 * 1		| first data block (usualy with 1st block of dir entries)
 * ...      | other dir blocks and files data blocks
 */

#define BLOCKSZ (DISK_BLOCK_SIZE)
#define SBLOCK 0          // superblock is at disk block 0
#define FS_MAGIC (0xf0f0) // for OFS
#define FNAMESZ 11        // file name size
#define LABELSZ 12        // disk label size
#define MAXDIRSZ 504      // max entries in the directory (1024-4-LABELSZ)/2

#define DIRENTS_PER_BLOCK (BLOCKSZ / sizeof(struct fs_dirent))
#define FBLOCKS 8 // 8 block indexes in each dirent

/* dirent .st field values: */
#define TFILE 0x10  // is file dirent
#define TEMPTY 0x00 // not used/free
#define TEXT 0xff   // is extent

#define FALSE 0
#define TRUE 1

#define FREE 0
#define NOT_FREE 1

/*** FSO Old/Our FileSystem disk layout ***/

struct fs_dirent { // a directory entry (dirent/extent)
    uint8_t st;
    char name[FNAMESZ];
    uint16_t ex; // numb of extra extents or id of this extent
    uint16_t ss; // number of bytes in the last extent (can be this dirent)
    uint16_t
        blocks[FBLOCKS]; // disk blocks with file content (zero value = empty)
};

struct fs_sblock { // the super block
    uint16_t magic;
    uint16_t fssize;        // total number of blocks (including this sblock)
    char label[LABELSZ];    // disk label
    uint16_t dir[MAXDIRSZ]; // directory blocks (zero value = empty)
};

/**
 * Nota: considerando o numero de ordem dos dirent em todos os blocos da
 * directoria um ficheiro pode ser identificado pelo numero do seu dirent. Tal
 * e' usado pelo open, create, read e write.
 */

union fs_block { // generic fs block. Can be seen with all these formats
    struct fs_sblock super;
    struct fs_dirent dirent[DIRENTS_PER_BLOCK];
    char data[BLOCKSZ];
};

/*******************************************/

struct fs_sblock superB; // superblock of the mounted disk

uint8_t *blockBitMap; // Map of used blocks (not a real bitMap, more a byteMap)
                      // this is build by mount operation, reading all the directory

/*******************************************/
/* The following functions may be usefull
 * change these and implement others that you need
 */

/**
 * allocBlock: allocate a new disk block
 * return: block number
 */
int allocBlock() {
    int i;

    for (i = 0; i < superB.fssize && blockBitMap[i] == NOT_FREE; i++)
        ;
    if (i < superB.fssize) {
        blockBitMap[i] = NOT_FREE;
        return i;
    } else
        return -1; // no disk space
}

/**
 */
void freeBlock(int nblock) {
    blockBitMap[nblock] = FREE;
}


/**
 * writes the super block to disk
 */
void write_metadata(){
    union fs_block block;
    block.super = superB;
    disk_write(SBLOCK, block.data);
}

/**
 * copy str to dst, converting from C string to FS string:
 *   - uppercase letters and ending with spaces
 * dst and str must exist with at least len chars
 */
void strEncode(char *dst, char *str, int len) {
    int i;
    for (i = 0; i < len && str[i] != '\0'; i++)
        if (isalpha(str[i]))
            dst[i] = toupper(str[i]);
        else if (isdigit(str[i]) || str[i] == '_' || str[i] == '.')
            dst[i] = str[i];
        else
            dst[i] = '?'; // invalid char?
    for (; i < len; i++)
        dst[i] = ' '; // fill with space
}

/**
 * copy str to dst, converting from FS string to C string
 * dst must exist with at least len+1 chars
 */
void strDecode(char *dst, char *str, int len) {
    int i;
    for (i = len - 1; i > 0 && str[i] == ' '; i--)
        ;
    dst[i + 1] = '\0';
    for (; i >= 0; i--)
        dst[i] = str[i];
}

/**
 * print super block content to stdout (for debug)
 */
void dumpSB() {
    union fs_block block;
    char label[LABELSZ + 1];

    disk_read(SBLOCK, block.data);
    printf("superblock:\n");
    printf("    magic = %x\n", block.super.magic);
    printf("    %d blocks\n", block.super.fssize);
    printf("    dir_size: %d\n", MAXDIRSZ);
    printf("    first dir block: %d\n", block.super.dir[0]);
    strDecode(label, block.super.label, LABELSZ);
    printf("    disk label: %s\n", label);

    printf("dir blocks: ");
    for (int i = 0; block.super.dir[i] != 0; i++)
        printf("%d ", block.super.dir[i]);
    putchar('\n');
}

/**
 * search and read file dirent/extent:
 * 	if ext==0: find 1st entry (with .st=TFILE)
 * 	if ext>0:  find extent (with .st=TEXT) and .ex==ext
 *  if ent!=NULL fill it with a copy of the dirent/extent
 *  return dirent index in the directory (or -1 if not found)
 */
int readFileEntry(char *name, uint16_t ext, struct fs_dirent *ent) {
    union fs_block block;

    for (int dirblk = 0; dirblk < MAXDIRSZ && superB.dir[dirblk]; dirblk++) {
        int b = superB.dir[dirblk];
        disk_read(b, block.data);
        for (int j = 0; j < DIRENTS_PER_BLOCK; j++)
            if ((((ext == 0 && block.dirent[j].st == TFILE)) &&
                 strncmp(block.dirent[j].name, name, FNAMESZ) == 0) ||
                (((block.dirent[j].st == TEXT && block.dirent[j].ex == ext)) &&
                 strncmp(block.dirent[j].name, name, FNAMESZ) == 0)) {
                if (ent != NULL)
                    *ent = block.dirent[j];
                return dirblk * DIRENTS_PER_BLOCK + j; // this dirent index
            }
    }

    return -1;
}

/**
 * update dirent at idx with 'entry' or, if idx==-1, add a new dirent to
 * directory with 'entry' content.
 * return: idx used/allocated, -1 if error (no space in directory)
 */
int writeFileEntry(int idx, struct fs_dirent entry) {

    // update dirent idx or allocate a new one
    // and write to directory on disk

    union fs_block block;

    if(idx != -1){
        int diskblk = superB.dir[idx/DIRENTS_PER_BLOCK]; // dirent disk location
        disk_read(diskblk, block.data);
        block.dirent[idx % DIRENTS_PER_BLOCK] = entry; // updates the dirent at idx
        disk_write(diskblk, block.data);
    }else{
        
        //search for the first free dirent slot
        int dirblk;
        for (dirblk = 0; dirblk < MAXDIRSZ && superB.dir[dirblk]; dirblk++) {
            int b = superB.dir[dirblk];
            disk_read(b, block.data);
            for (int j = 0; j < DIRENTS_PER_BLOCK; j++){
                if(block.dirent[j].st == TEMPTY){
                    block.dirent[j]= entry;
                    disk_write(b, block.data);
                    write_metadata();
                    return dirblk * DIRENTS_PER_BLOCK + j; // this dirent index
                }
            }
        }
        //alocate new block for dirents
        int newblk;
        if(dirblk == MAXDIRSZ || (newblk = allocBlock()) == -1) return -1;
        superB.dir[dirblk] = newblk;
        write_metadata();
        memset(&block, 0, BLOCKSZ);
        block.dirent[0] = entry;
        disk_write(newblk, block.data);
        idx =  dirblk * DIRENTS_PER_BLOCK;
    }
    return idx;
}

/****************************************************************/

/**
 * Marks the entry as free (TEMPTY)
 * frees all the blocks used by teh entry
 */
void free_entry(struct fs_dirent *entry){
    entry->st = TEMPTY;
    int file_block_num;
    for(int i = 0; i < FBLOCKS && (file_block_num = entry->blocks[i]) ; i++)
        freeBlock(file_block_num);
}


int fs_delete(char *name) {

    if (superB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    char fname[FNAMESZ];
    strEncode(fname, name, FNAMESZ);

    struct fs_dirent entry;
    int idx;
    int ext = -1;
    for(int i = 0; ext == -1 || i <= ext; i++){
        idx = readFileEntry(fname, i, &entry);
        if(idx == -1) return -1; // entry not found
        if(ext == -1) ext = entry.ex; // first extends
        // free entry and write changes to disk
        free_entry(&entry);
        writeFileEntry(idx, entry);
    }
    return 0;
}


/*****************************************************/

void fs_dir() {

    if (superB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return;
    }
    
    union fs_block block;
    for (int dirblk = 0; dirblk < MAXDIRSZ && superB.dir[dirblk]; dirblk++) {
        int b = superB.dir[dirblk];
        disk_read(b, block.data);
        for (int j = 0; j < DIRENTS_PER_BLOCK; j++){
            struct fs_dirent currentDirent =  block.dirent[j];
            if(currentDirent.st == TFILE){
                unsigned dirent_number = dirblk*DIRENTS_PER_BLOCK + j;  // this dirent index
                char file_name[FNAMESZ + 1];  // to store the file name as a C string hence the +1 for the '\0' char                                  
                strDecode(file_name, currentDirent.name , FNAMESZ);                 
                unsigned file_size = (currentDirent.ex * (BLOCKSZ * FBLOCKS)) + currentDirent.ss; // total size of the file including the extends
                printf( "%u: %s, size: %u bytes\n", dirent_number, file_name, file_size);
            }
        }
    }
}

/*****************************************************/

void fs_debug() {
    union fs_block block;

    disk_read(SBLOCK, block.data);

    if (block.super.magic != FS_MAGIC) {
        printf("disk unformatted !\n");
        return;
    }
    dumpSB();

    printf("**************************************\n");
    if (superB.magic == FS_MAGIC) {
        printf("Used blocks: ");
        for (int i = 0; i < superB.fssize; i++) {
            if (blockBitMap[i] == NOT_FREE)
                printf(" %d", i);
        }
        puts("\nFiles:\n");
        fs_dir();
    }
    printf("**************************************\n");
}

/*****************************************************/

int fs_format(char *disklabel) {
    union fs_block block;
    int nblocks;

    assert(sizeof(struct fs_dirent) == 32);
    assert(sizeof(union fs_block) == BLOCKSZ);

    if (superB.magic == FS_MAGIC) {
        printf("Cannot format a mounted disk!\n");
        return 0;
    }
    if (sizeof(block) != DISK_BLOCK_SIZE) {
        printf("Disk block and FS block mismatch\n");
        return 0;
    }
    memset(&block, 0, sizeof(block));
    disk_write(1, block.data); // write 1st dir block all zeros

    nblocks = disk_size();
    block.super.magic = FS_MAGIC;
    block.super.fssize = nblocks;
    strEncode(block.super.label, disklabel, LABELSZ);
    block.super.dir[0] = 1; // block 1 is first dir block

    disk_write(0, block.data);  // write superblock
    dumpSB(); // debug

    return 1;
}

/*****************************************************************/

int fs_mount() {
    union fs_block block;

    if (superB.magic == FS_MAGIC) {
        printf("One disc is already mounted!\n");
        return 0;
    }
    disk_read(SBLOCK, block.data);
    superB = block.super;

    if (superB.magic != FS_MAGIC) {
        printf("cannot mount an unformatted disc!\n");
        return 0;
    }
    if (superB.fssize != disk_size()) {
        printf("file system size and disk size differ!\n");
        return 0;
    }

    // build used blocks map
    blockBitMap = malloc(superB.fssize * sizeof(uint16_t));
    memset(blockBitMap, FREE, superB.fssize*sizeof(uint16_t));
    blockBitMap[0] = NOT_FREE; // 0 is used by superblock


    for (int dirblk = 0; dirblk < MAXDIRSZ && superB.dir[dirblk]; dirblk++) {
        int b = superB.dir[dirblk];
        blockBitMap[b] = NOT_FREE; // block user for dirents
        disk_read(b, block.data);
        for (int j = 0; j < DIRENTS_PER_BLOCK; j++){
            struct fs_dirent currentDirent =  block.dirent[j];
            if(currentDirent.st != TEMPTY){
                int file_block_num; // number of a block used by the file
                for(int i = 0; i < FBLOCKS && (file_block_num = currentDirent.blocks[i]) ; i++)
                    blockBitMap[file_block_num] = NOT_FREE; // block used by the file
            }
        }
    }

    return 1; 
}

/************************************************************/

int fs_read(char *name, char *data, int length, int offset) {

    if (superB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    
    char fname[FNAMESZ];
    strEncode(fname, name, FNAMESZ);
 
    struct fs_dirent entry;
    if(readFileEntry(fname, 0, &entry) == -1) return -1; //file not found

    int file_size = (entry.ex * (BLOCKSZ * FBLOCKS)) + entry.ss; 

    if (offset > file_size) return 0;
    
    if(offset + length > file_size)
        length = file_size - offset;
    
    int ext = offset / (BLOCKSZ * FBLOCKS); //extent number
    int current_entry_block = (offset / BLOCKSZ) % FBLOCKS; //block of the entry
    int inblock_startbyte = offset - (ext * BLOCKSZ * FBLOCKS) - (current_entry_block * BLOCKSZ); //offset in the block

    if(ext != 0) 
        readFileEntry(fname, ext, &entry);
    

    union fs_block block; 
    int inblock_read; //sum of bytes read in a block
    int total_read = 0; //sum of read bytes
    

    while(total_read < length){

        if(total_read != 0) inblock_startbyte = 0; //set inblock_startbyte to 0 after first read 

        if(current_entry_block == FBLOCKS){
            current_entry_block = 0;
            ext++;
            readFileEntry(fname, ext, &entry); //always != -1 here
        }
        
        disk_read(entry.blocks[current_entry_block++], block.data); 
        inblock_read = MIN(length - total_read, BLOCKSZ - inblock_startbyte);
        memcpy(data + total_read, block.data + inblock_startbyte, inblock_read);
        total_read += inblock_read;

    }
    
    return total_read;


    /*

    ----> menos eficiente

    int block_pos = inblock_offset;
    int data_pos;
   

    for(data_pos = 0; data_pos < length; data_pos++){
        
        if(block_pos == BLOCKSZ){
            block_pos = 0;
            entry_block ++;
            if(entry_block == FBLOCKS){
                entry_block = 0;
                ext++;
                readFileEntry(fname, ext, &entry);
            }
            disk_read(entry.blocks[entry_block], block.data);
        }

        data[data_pos] = block.data[block_pos++];
    }
    return data_pos;
    */
}

/****************************************************************/

int fs_write(char *name, char *data, int length, int offset) {

    if (superB.magic != FS_MAGIC) {
        printf("disc not mounted\n");
        return -1;
    }
    char fname[FNAMESZ];
    strEncode(fname, name, FNAMESZ);


    // find TFILE file entry or create one
    struct fs_dirent entry;
    int idx = readFileEntry(fname, 0, &entry); 

    if(idx == -1){ //if not found create file  
        memset(&entry , 0, sizeof(struct fs_dirent));
        entry.st = TFILE;
        strncpy(entry.name , fname, FNAMESZ);
        idx = writeFileEntry(idx, entry);
        if(idx == -1) return -1; // no space in directory  
    }

    if(length == 0)
        return 0;


    int file_size =  entry.ex * (BLOCKSZ * FBLOCKS) + entry.ss;
    if( offset > file_size) //offset out of bounds
        return -1;


    int ext = offset / (BLOCKSZ * FBLOCKS); //extent number
    int current_entry_block = (offset / BLOCKSZ) % FBLOCKS; //block of the entry
    int inblock_startbyte = offset - (ext * BLOCKSZ * FBLOCKS) - (current_entry_block * BLOCKSZ); //offset in the block
    
    
    if(ext != 0) 
        idx = readFileEntry(fname, ext, &entry);
     
    
    union fs_block block;
    int inblock_writen;
    int total_writen = 0; // sum of the total bytes writen


    while(total_writen < length){

        if(total_writen != 0) inblock_startbyte = 0; //set inblock_startbyte to 0 after first write

        //block and extends jump
        if(current_entry_block == FBLOCKS){
            current_entry_block = 0;
            ext++;
            idx = readFileEntry(fname, ext, &entry); 
            if(idx == -1){
                memset(&entry , 0, sizeof(struct fs_dirent));
                entry.st = TEXT;
                entry.ex = ext;
                strncpy(entry.name , fname, FNAMESZ);
                idx = writeFileEntry(idx, entry);
                if(idx == -1){
                    ext--;
                    break; // no disk space
                } 
            }
        }

        //file block alocation
        if(entry.blocks[current_entry_block] == 0){
            int newblk = allocBlock();
            if(newblk == -1){
                idx = -1;
                break; // no disk space
            }
            entry.blocks[current_entry_block] = newblk;
            //update the entry with the new block
            writeFileEntry(idx, entry); 
        }

        disk_read(entry.blocks[current_entry_block], block.data);
        inblock_writen = MIN(length - total_writen, BLOCKSZ - inblock_startbyte);
        memcpy(block.data + inblock_startbyte, data + total_writen, inblock_writen);
        total_writen += inblock_writen;
        disk_write(entry.blocks[current_entry_block++], block.data);

    }
    
    int tfile_idx = readFileEntry(fname, 0, &entry);
    entry.ex = ext;

    if (offset + total_writen > file_size)
        entry.ss = (offset + total_writen) % (BLOCKSZ * FBLOCKS);

    writeFileEntry(tfile_idx, entry); //update first entry TFILE

    return idx == -1 ? -1 : total_writen; //total bytes writen or -1

}
