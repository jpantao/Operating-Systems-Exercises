//Ficheiro para tratamento dos resultados das contagens

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include "process.h"


/**
 * process the replies arraiving from all pipes
 * should read each pipe (any pipe) as soon data
 * is available in it (or if pipe is closed)
 * in: nprocs = number of workers (size of pipes array)
 *     pipes = filedescriptors for all pipes connected to workers
 * return: sum of all counters received
 */
int process_workers_reply(int nprocs, int pipes[nprocs]) {
	int count = 0;

	fd_set mask;

	

	int received = 0; 

	while(received < nprocs) {
		
		int nfds = -1; // highest-numbered file descriptor
		FD_ZERO(&mask);
		for(int i = 0; i < nprocs; i++){
			if(pipes[i] != -1){
				FD_SET(pipes[i], &mask);
				if(pipes[i] > nfds)
					nfds = pipes[i];
			}
		}
		
		select(nfds + 1 ,  &mask, NULL, NULL, NULL);
		
		int reply;
		int rd;
		
		for(int i = 0; i < nprocs; i++){
			
			if( pipes[i] != -1 && FD_ISSET(pipes[i], &mask)){
				rd = read(pipes[i], &reply, sizeof(int));	
			
				if(rd == 0)
					printf("pipe to worker %d was closed\n", i);
				else{
					count += reply;
					printf("reply received from worker %d count: %d\n", i, reply);
				} 
				pipes[i] = -1;
				received ++;
			}
			
		}
	

	}

	return count;
}
