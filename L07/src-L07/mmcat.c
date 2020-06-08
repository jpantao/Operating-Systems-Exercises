#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

void fatal_error(char *str){
  perror(str);
  exit(1);
}

/* mmapcopy - uses mmap to copy file fd to stdout 
 */
void mmapcopy(int fd, int size) {
    char *bufp; /* ptr to memory mapped VM area */
    int n;

    bufp = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if( bufp == MAP_FAILED) fatal_error("mmap");

    n = write(1, bufp, size);
    if( n != size) fatal_error("write");
}

int main(int argc, char *argv[]) {
    struct stat stat;
    int fd;

    /* check for required command line argument */
    if (argc != 2) {
	printf("usage: %s <filename>\n", argv[0]);
	exit(1);
    }

    /* copy the input argument to stdout */
    fd = open(argv[1], O_RDONLY, 0);
    if( fd < 0) fatal_error("open");

    if( fstat(fd, &stat) != 0) fatal_error("fstat");
	
    mmapcopy(fd, stat.st_size);
    return 0;
}

