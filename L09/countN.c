#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>

#define SIZE (200*1024*1024)

int *array;
int tofind = 3;
int slice;




void * docount( void* arg ){
	int *arr = (int*) arg;
	int ret=0;
	for (int i=0; i < slice; i++) {
		if(arr[i] == tofind){
			ret++;
		}
	}
	return (void*) ret;
}


int main( int argc, char *argv[]){
	struct timeval t1,t2;

	int count = 0;
	int nThreads = atoi(argv[1]);
	pthread_t threads[nThreads];
	int slice = SIZE/nThreads;
	array= (int *)malloc(SIZE*sizeof(int));
	tofind = 3;

	srand(0);
	for (int i=0; i < SIZE; i++) {
		array[i] = rand() % 4;
	}

	gettimeofday(&t1, NULL);
	
		
	for(int i = 0; i < nThreads; i++)
		pthread_create(&threads[i], NULL, docount, (void*)(array+i*slice));

	int ret = 0;
	for(int i = 0; i < nThreads; i++){
		pthread_join(threads[i], (void*) &ret);
		count += ret;
	}
	gettimeofday(&t2, NULL);


	printf("Count of %d = %d\n", tofind, count);
	printf("Elapsed time (ms) = %lf\n", 
		((t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec))/1000.0 );

	return 0;
}
