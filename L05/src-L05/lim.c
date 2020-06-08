
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

void Getrlimit( int limitType, struct rlimit *r){
	int res = getrlimit( limitType, r);
	if(res < 0){ perror("getrlimit"); exit(1);}
}

int main(int argc, char *argv[])
{
    struct rlimit r;
    char a[9*1024*1024]; // Segmentation fault 
    

    Getrlimit( RLIMIT_AS, &r);
    printf("Maximum size of process virtual addres space; soft limit = %lx, hard limit = %lx\n", 
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);

    Getrlimit( RLIMIT_DATA, &r);
    printf("Maximum size of process's data segment(intialized, uninitiliazed, heap); soft limit = %lx, hard limit = %lx\n",
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);

    Getrlimit( RLIMIT_STACK, &r);
    printf("Maximum size of process stack; soft limit = %lx, hard limit = %lx\n", 
               (unsigned long)r.rlim_cur, (unsigned long)r.rlim_max);
    return 0;
}

