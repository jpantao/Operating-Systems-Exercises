#ifndef PTHREAD_TO_POS
#define PTHREAD_TO_POS

/**
 * Returns the mapping of id onto a referential starting in 0.
 * Must be used to convert a pthread_id into a index of an array
 */
int pthread_to_pos(pthread_t id);

#endif