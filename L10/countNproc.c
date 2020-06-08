/**
 * countNproc - Lab 10 FSO 1819  MIEI
 * DI - FCT/UNL
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/wait.h>



#define SIZE    (200*1024*1024)

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

int launchWorker( int n, int size ) { // returns the pipe stream to read result from
    // TODO
    int filedes[2];
    int status;
    status = pipe(filedes);
    
    if(status == -1){
        exit(1); // erro
    }

    switch(fork()){
        case -1:
            exit(1); // erro
        case 0: /* Child - writes to pipe */
            close(filedes[0]);
            int c = docount(n, size);
            write(filedes[1], &c, sizeof(c));
            exit(0);
        default: /* Parent - reads from pipe */
            close(filedes[1]);
    }
           
    return filedes[0];
    
}


int main(int argc, char const *argv[])
{
	struct timeval t1,t2;
	int size, count = 0;

    if ( argc!=2 ) {
        fprintf(stderr,"usage: %s num_procs\n", argv[0]);
        return 1;
    }
    int nprocs = atoi( argv[1] );
    printf("using %d procs\n", nprocs);

	array= (int *)malloc(SIZE*sizeof(int)); // global array
	tofind = 3;
    size = SIZE/nprocs;

	srand(0);
	for (int i=0; i < SIZE; i++) {
		array[i] = rand() % 4;
	}

    int *pipes=alloca(nprocs*sizeof(int));  // local array

	gettimeofday(&t1, NULL);
    for ( int p=0; p<nprocs && pipes[p-1]!=-1; p++ ) {
        pipes[p] = launchWorker( p, size );  // workers
        assert( pipes[p]!=-1 );
    }
    for ( int p=0; p<nprocs; p++ ) {
        int c;
        read(pipes[p], &c, sizeof(c));
        count += c;
    }
  	gettimeofday(&t2, NULL);
    for ( int p=0; p<nprocs; p++ )
        wait(NULL);

	printf("Count of %d = %d\n", tofind, count);
	printf("Elapsed time (ms) = %lf\n", 
		((t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec))/1000.0 );
    return 0;
}
