#ifndef FS_H
#define FS_H

void fs_debug();

void fs_dir();

int  fs_format();

int  fs_mount();

/**
 * deletes the named file. If not found do nothing.
 */
int  fs_delete( char *fsname );

/**
 * tries to read to data, length bytes, from offset forward. 
 * file name to read is 'name'.
 * returns the number of bytes read (zero when there is no more bytes to read)
 */ 
int  fs_read( char *name, char *data, int length, int offset );

/**
 * writes length bytes from data to file offset forward.
 * file name to write into is 'name'.
 * returns the number of bytes writen (zero when there is no more space)
 */
int  fs_write( char *name, char *data, int length, int offset );

#endif
