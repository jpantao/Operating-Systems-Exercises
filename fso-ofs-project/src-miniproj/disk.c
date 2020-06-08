#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "disk.h"

#define DEBUG

static FILE *diskfile;
static unsigned int nblocks = 0;

// stats:
static unsigned int nreads = 0;
static unsigned int nwrites = 0;

int disk_init( const char *filename, int n ) {
    diskfile = fopen( filename, "r+" );
    if (diskfile != NULL) { // disk exists
        fseek( diskfile, 0L, SEEK_END );
        n = ftell( diskfile );
        n = n / DISK_BLOCK_SIZE;
    } else if ( n>1 ) {
        diskfile = fopen( filename, "w+" );
        ftruncate( fileno( diskfile ), n * DISK_BLOCK_SIZE );
    }
    if (diskfile==NULL)
        return 0;
    
    printf( "disksize=%d (%d blocks)\n", n*DISK_BLOCK_SIZE, n );
    nblocks = n;
    nreads = 0;
    nwrites = 0;

    return 1;
}

unsigned int disk_size( ) {
    return nblocks;
}

static void sanity_check( unsigned int blocknum, const void *data ) {
    if (blocknum >= nblocks) {
        printf( "ERROR: blocknum (%d) is too big!\n", blocknum );
        abort();
    }

    if (!data) {
        printf( "ERROR: null data pointer!\n" );
        abort();
    }
}

void disk_read( unsigned int blocknum, char *data ) {
    sanity_check( blocknum, data );

    if ( fseek( diskfile, blocknum * DISK_BLOCK_SIZE, SEEK_SET )<0 )
        perror("disk_read seek");

    if (fread( data, DISK_BLOCK_SIZE, 1, diskfile ) == 1) {
        nreads++;
    } else {
        printf( "ERROR: couldn't access simulated disk: %s\n",
                strerror( errno ) );
        abort();
    }
}

void disk_write( unsigned int blocknum, const char *data ) {
#ifdef DEBUG
    printf( "Writing block %d\n", blocknum );
#endif
    sanity_check( blocknum, data );

    if ( fseek( diskfile, blocknum * DISK_BLOCK_SIZE, SEEK_SET )<0 )
        perror("disk_write seek");

    if (fwrite( data, DISK_BLOCK_SIZE, 1, diskfile ) == 1) {
        nwrites++;
    } else {
        printf( "ERROR: couldn't access simulated disk: %s\n",
                strerror( errno ) );
        abort();
    }
}

void disk_close( ) {
    if (diskfile) {
        printf( "%d disk block reads\n", nreads );
        printf( "%d disk block writes\n", nwrites );
        fclose( diskfile );
        diskfile = 0;
    }
}

