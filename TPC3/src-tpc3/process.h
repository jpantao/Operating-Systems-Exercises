

/**
 * process the replies arraiving from all pipes
 * should read each pipe (any pipe) as soon data
 * is available in it (or if pipe is closed)
 * in: nprocs = number of workers (size of pipes array)
 *     pipes = filedescriptors for all pipes connected to workers
 * return: sum of all counters received
 */
int process_workers_reply(int nprocs, int pipes[nprocs]);
