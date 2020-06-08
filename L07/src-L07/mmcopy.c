#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <string.h>

void fatal_error(char *str){
  perror(str);
  exit(1);
}

/* mmapcopy - uses mmap to copy file fd1 to fd2
 */
void mmapcopy(int fd1, int fd2, int size, int bsize) {
    char *bufp1; /* ptr to memory mapped VM area */
    char *bufp2;
    int offset = 0;

    if (size == 0 ) return;

    if (bsize % getpagesize() != 0){
        bsize = bsize + getpagesize() - (bsize % getpagesize());
    } 

    ftruncate(fd2, size);

    while (offset < size){

        if( (offset + bsize) > size) bsize = size - offset;

        bufp1 = mmap(NULL, bsize, PROT_READ, MAP_PRIVATE, fd1, offset);
        if( bufp1 == MAP_FAILED) fatal_error("mmap 1");

        bufp2 = mmap(NULL, bsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, offset);
        if( bufp2 == MAP_FAILED) fatal_error("mmap 2");

        memcpy(bufp2, bufp1, size);
        munmap(bufp1, bsize);
        munmap(bufp2, bsize);
        offset+= bsize;
    }
}

int main(int argc, char *argv[]) {
    struct stat stat;
    int fd1;
    int fd2;
    

    /* check for required command line arguments */
    if (argc != 4) {
	    printf("usage: %s size <filename> <filename>\n", argv[0]);
	    exit(1);
    }

    
    fd1 = open(argv[2], O_RDONLY, 0);
    if( fd1 < 0) fatal_error("open");

    if( fstat(fd1, &stat) != 0) fatal_error("fstat");

    fd2 = open(argv[3], O_RDWR|O_CREAT|O_TRUNC, 0600);
    if( fd2 < 0) fatal_error("open output");
	
    mmapcopy(fd1, fd2, stat.st_size, atoi(argv[1]));
    return 0;
}

