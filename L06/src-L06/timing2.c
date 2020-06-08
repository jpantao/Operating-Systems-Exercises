#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



void do_something( long int trials, long int numpag, int* a ) {
    int jump = getpagesize() / sizeof(int);
    for (int i = 0; i < trials; i++){
        for (int p = 0; p <numpag * jump; p+=jump){
            a[p] += 1;
        }
    }
}


int main(int argc, char *argv[]) {
    int i, p;
    long elapsed;
    struct timeval t1,t2;

	if ( argc!=3 ) {
		fprintf(stderr, "usage: %s trials pages\n", argv[0]);
		exit(1);
	}

	long int trials = atol(argv[1]);
	long int numpag = atol(argv[2]);
    int *a = (int*) malloc(getpagesize() * numpag);

    gettimeofday(&t1, NULL);
	do_something( trials, numpag, a);
    gettimeofday(&t2, NULL);

    free(a);

    elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
    printf("%ld pages %g \n", numpag, (double)elapsed/(double)(trials*numpag));
    return 0;
}


