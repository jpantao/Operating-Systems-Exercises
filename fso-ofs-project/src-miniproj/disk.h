#ifndef DISK_H
#define DISK_H

/**
 * Disk device interface
 * Only one disk at a time supported.
 */

#define DISK_BLOCK_SIZE 1024


int  disk_init( const char *filename, int nblocks );
unsigned int  disk_size();
void disk_read( unsigned int blocknum, char *data );
void disk_write( unsigned int blocknum, const char *data );
void disk_close();


#endif
