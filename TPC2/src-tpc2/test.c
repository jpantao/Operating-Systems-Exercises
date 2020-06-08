#include "fso_phaser.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const int sleep_time = 100;
const int items_per_thread = 100;

fso_phaser_t my_phaser;


void* some_computation(void* arg) {

	for (int i = 0; i < items_per_thread; i++) {
		usleep(sleep_time);
		fso_phaser_advance(&my_phaser);
	}

	return NULL;	
}

int main(int argc, char** argv) {
	
	if (argc != 2) {
		printf ("usage %s number_threads\n", argv[0]);
		return -1;
	}	

	const int nthreads = atoi(argv[1]);
	const int total_items = nthreads * items_per_thread;

	fso_phaser_init(&my_phaser, nthreads+1);

	pthread_t threads[nthreads];

	for (int i = 0; i < nthreads; i++) 
		pthread_create( &threads[i], NULL, some_computation, NULL); 


	printf ("Progress:\n");
	for (int completed = 0; completed < total_items; completed += nthreads) {
		printf ("%d%% ...", (completed*100/total_items));
		fflush(stdout);
		fso_phaser_advance_and_await(&my_phaser);
	}

	printf ("done\n");


	for (int i = 0; i < nthreads; i++) 
		pthread_join(threads[i], NULL); 

	fso_phaser_destroy(&my_phaser);

	return 0;
}