
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *Sbrk(unsigned int increment) {
    void *res = sbrk((intptr_t)increment);
    if (res == (void *)-1) {
        perror("sbrk");
        exit(1);
    }
    return res;
}

int main(int argc, char *argv[]) {
    unsigned long moreSpace;

    if (argc != 2) {
        printf("%s size_of_memory_to_allocate\n", argv[0]);
        exit(1);
    }
    moreSpace = atol(argv[1]);

    // getting the current break value
    printf("Current program break = %lu\n", (unsigned long)Sbrk(0));

    // moving program break
    Sbrk(moreSpace);
    printf("New program break = %lu\n", (unsigned long)Sbrk(0));

    return 0;
}
