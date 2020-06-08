/**
 * countNprocMain - TPC3 - FSO 1819  MIEI
 * DI - FCT/UNL
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "process.h"


#define SIZE    (100*1024*1024)

int *array;  //data to work on (we are expecting that the OS will
			 // use shared pages among all child processes)
int tofind;


int docount( int n, int size ){
	int count = 0;
    int start=n*size;
    int end=(n+1)*size;
    for (int i=start; i < end; i++) {
		if(array[i] == tofind)
			count++;
	}
    return count;
}

int launchWorker( int n, int size, int seed ) { // returns the pipe stream to read result from
    int p[2];
    pid_t pid;
    if (pipe(p) == -1)
        perror("pipe");
    else if ((pid = fork()) == 0) {
        close(p[0]);
        int c = docount(n, size);
        srand(seed + n);
        usleep(rand() % 250000);	// just for testing
        write(p[1], &c, sizeof(int));
        exit(0);
    } else if (pid == -1)
        perror("fork");
    else {
        close(p[1]);
        return p[0];
    }
    return -1;
}

int main(int argc, char const *argv[])
{
	int size, seed, count = 0;

    if ( argc!=3 ) {
        fprintf(stderr,"usage: %s num_procs seed_for_random\n", argv[0]);
        return 1;
    }
    int nprocs = atoi( argv[1] );
    printf("using %d procs\n", nprocs);
    seed = atoi(argv[2]);

	array= (int *)malloc(SIZE*sizeof(int)); // global array
	tofind = 3;
    size = SIZE/nprocs;

	srand(0);
	for (int i=0; i < SIZE; i++) {
		array[i] = rand() % 4;
	}

    int *pipes=alloca(nprocs*sizeof(int));  // local array

    for ( int p=0; p<nprocs; p++ ) {
        pipes[p] = launchWorker( p, size, seed );  // workers
        assert( pipes[p]!=-1 );
    }
 
    count = process_workers_reply(nprocs, pipes);
    
    for ( int p=0; p<nprocs; p++ )
        wait(NULL);
    printf("Count of %d = %d\n", tofind, count);
    return 0;
}
