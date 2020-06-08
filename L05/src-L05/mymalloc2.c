
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCK_SIZE sizeof(struct s_block)

typedef struct s_block *t_block;

struct s_block {
    size_t size;  // current size of block
    t_block next; // pointer to next block
    int free;     // flag indicating that the block is free or occupied; 0 or 1.
              // Occupies 32 bits but the compiler aligns the structure to a
              // multiple of 4
};

t_block base = NULL; // points to the beginning of the list
t_block last = NULL; // points to the last element of the list


t_block find_block(size_t size) {
    t_block b = base;
    while (b != NULL && !(b->free && b->size >= size)) {
        b = b->next;
    }
    return b;
}

t_block extend_heap(size_t s) {
    t_block b = sbrk(BLOCK_SIZE + s);
    if ( b == (void *)-1)
        return NULL; /* if sbrk fails, return NULL pointer*/

    b->size = s;
    b->next = NULL;
    b->free = 0;
    if (base==NULL) base = b;
    else last->next = b;
    last = b;
    return b;
}

void debugBlockList() {
    t_block p = base;
    printf("current block list:\n");
    while(p != NULL){
        printf("size:%lu free:%d\n", p->size, p->free);
        p = p->next;
    }
    printf("\n");


}

void *myMalloc(size_t size) {
    t_block p = find_block(size);
    if(p == NULL)
       p = extend_heap(size); 
    else
       p->free = 0;
    return p + 1;
}

void myFree(void *ptr) {
    t_block p = (t_block) ptr - 1;
    p -> free = 1;
}



/**
 * teste myMalloc/myFree
 **/
int main(int argc, char *argv[]) {
    unsigned int maxSpace;

    if (argc != 2) {
        printf("%s max_memory_to_allocate\n", argv[0]);
        exit(1);
    }
    maxSpace = atoi(argv[1]);

    // getting the current break value
    printf("Current program break = %lu\n", (unsigned long)sbrk(0));

    for (int s = 1; s < maxSpace; s *= 2) {
        void *ptr = myMalloc(s);
        if (ptr == NULL)
            printf("No more mem\n");
        else
            printf("returned pointer = %lu\n", (unsigned long)ptr);
        myFree(ptr);
    }
    debugBlockList();
    for (int s = 1; s < maxSpace; s *= 2) {
        void *ptr = myMalloc(s);
        if (ptr == NULL)
            printf("No more mem\n");
        else
            printf("returned pointer = %lu\n", (unsigned long)ptr);
    }
    debugBlockList();

    return 0;
}
