
#ifndef FSO_PHASER
#define FSO_PHASER

#include <pthread.h>

struct fso_phaser {
	pthread_mutex_t mutex; // The mutex for exclusive access to the phaser
	pthread_cond_t cond; // The condition variable for signaling phase reaching and waiting for other threads.
	int number_threads; // Number of threads using the phaser for synchronization.
	int* phases; // Array with the phase of each thread in the phaser.
};

typedef struct fso_phaser fso_phaser_t; 


/**
 * Creates a new fso_phaser for the specified number of threads.
 * If successful, returns zero and puts the new fso_phaser in phaser, 
 * otherwise it returns an error number to indicate the error.
 */
int fso_phaser_init(fso_phaser_t* phaser, int number_threads);

/**
 * Increments the thread's phase in the phaser
 * If successful, returns zero,
 * otherwise it returns -1 to indicate the error.
 */
int fso_phaser_advance(fso_phaser_t* phaser);

/**
 * Increments the thread's phase in the phaser and waits until the
 * remainder threads have also reached the same phase.
 * If successful, returns zero,
 * otherwise it returns -1 to indicate the error.
 */
int fso_phaser_advance_and_await(fso_phaser_t* phaser);

/**
 * Returns the current phase of the phaser, i.e.
 * the phase that all threads have already reached
 */
int fso_phaser_current(fso_phaser_t* phaser);

/**
 * Destroys the specified fso_phaser.
 * If successful, returns zero,
 * otherwise it returns an error number to indicate the error.
 */
int fso_phaser_destroy(fso_phaser_t* phaser);

#endif