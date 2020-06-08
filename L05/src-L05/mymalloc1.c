
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


void * myMalloc(size_t size){
	void *p;
	p = sbrk(size);
	/* If sbrk fails , we return NULL */
	if (p == (void*)-1) 
		return NULL;
    return p;
}


int main(int argc, char *argv[])
{
    unsigned int moreSpace;

    if( argc != 2){
	printf("%s size_of_memory_to_allocate\n", argv[0]);
	exit(1);
    }
    moreSpace = atoi(argv[1]);

    // getting the current break value
    printf("Current program break = %lu\n", (unsigned long) sbrk(0));

    char *mymem = (char*)myMalloc(moreSpace);
	if (mymem==NULL) printf("No more mem\n");
    else printf("Allocated mem at %lu. New program break = %lu\n", 
				(unsigned long)mymem, (unsigned long) sbrk(0));

    return 0;
}

