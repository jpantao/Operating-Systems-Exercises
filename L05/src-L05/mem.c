#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global;           // global var not inicialized
int vglob = 3;        // inicialized global var
const int cglob = 10; // global constant

int main(int argc, char *argv[]) {
    long pid; // local var (on stack)
    int *p;   // local var for pointer

    if (argc != 2) {
        fprintf(stderr, "usage: %s <value>\n", argv[0]);
        exit(1);
    }
    p = (int *)malloc(sizeof(int)); // malloc'd memory is on "heap"
    assert(p != NULL);

    *p = atoi(argv[1]);
    pid = (long)getpid();
    global = vglob = pid; // all vars written with process ID

    printf("(pid:%ld) addr of main:   %lx\n", pid, (unsigned long)main);
    printf("(pid:%ld) addr of printf: %lx\n", pid, (unsigned long)printf);
    printf("(pid:%ld) addr of getpid: %lx\n", pid, (unsigned long)getpid);
    printf("(pid:%ld) addr of p:      %lx\n", pid, (unsigned long)&p);
    printf("(pid:%ld) addr of argv:   %lx\n", pid, (unsigned long)argv);
    printf("(pid:%ld) addr stored in p: %lx\n", pid, (unsigned long)p);
    printf("(pid:%ld) addr of global: %lx\n", pid, (unsigned long)&global);
    printf("(pid:%ld) addr of cglob:  %lx\n", pid, (unsigned long)&cglob);
    printf("(pid:%ld) addr of vglob:  %lx\n", pid, (unsigned long)&vglob);

    while (*p > 0) {
        *p = *p - 1;
        sleep(10);
        printf("(pid:%ld) value of p: %d\n", pid, *p);
    }
    return 0;
}
