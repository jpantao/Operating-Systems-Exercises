
#include "fso_phaser.h"

#include <assert.h>

#include "pthread_to_pos.h"


#include <stdlib.h>
#include <string.h>


int fso_phaser_init(fso_phaser_t* phaser, int number_threads) {

	assert(number_threads > 0);

	int result;
	if ((result = pthread_mutex_init(&phaser->mutex, NULL)) < 0) 
		return result;

	if ((result = pthread_cond_init(&phaser->cond, NULL)) < 0) {
		pthread_mutex_destroy(&phaser->mutex);
		return result;
	}

	phaser->number_threads = number_threads;	
	phaser->phases = (int *) malloc(sizeof(int) * number_threads);
	memset(phaser->phases, 0, sizeof(int) * number_threads);
	return 0;		
}

int fso_phaser_current(fso_phaser_t* phaser) {

	int result = phaser->phases[0];

	for(int i = 1; i < phaser->number_threads; i++){
		if(phaser->phases[i] < result) 
			result = phaser->phases[i];
	}

	return result;	
}

int fso_phaser_advance_and_await(fso_phaser_t* phaser) {

	int result;

	
	pthread_mutex_lock(&(phaser->mutex));
	int index_in_phase_array = pthread_to_pos(pthread_self());
	pthread_mutex_unlock(&(phaser->mutex));

	if(index_in_phase_array > phaser->number_threads){
		result = -1;
	}else{
		//advance thread fase
		phaser->phases[index_in_phase_array]++;
		
		pthread_mutex_lock(&(phaser->mutex));
		while(fso_phaser_current(phaser) < phaser->phases[index_in_phase_array]){
			pthread_cond_wait(&(phaser->cond), &(phaser->mutex));
		}
		pthread_cond_broadcast(&(phaser->cond)); //signal this thread has advanced
		pthread_mutex_unlock(&(phaser->mutex));

		result = 0;
	}

	return result;
}

int fso_phaser_advance(fso_phaser_t* phaser) {

	int result;

	pthread_mutex_lock(&(phaser->mutex));
	int index_in_phase_array = pthread_to_pos(pthread_self());
	pthread_mutex_unlock(&(phaser->mutex));

	if(index_in_phase_array > phaser->number_threads){
		result = -1;
	}else{
		//advance thread fase
		phaser->phases[index_in_phase_array]++;
		pthread_mutex_lock(&(phaser->mutex));
		pthread_cond_broadcast(&(phaser->cond)); //signal this thread has advanced
		pthread_mutex_unlock(&(phaser->mutex));
		result = 0;
	}
	
	
	return result;
}

int fso_phaser_destroy(fso_phaser_t* phaser) {

	int result;
	if ((result = pthread_mutex_destroy(&phaser->mutex)) < 0) 
		return result;

	if ((result = pthread_cond_destroy(&phaser->cond)) < 0) 
		return result;

	free(phaser->phases);
	return 0;	
}
