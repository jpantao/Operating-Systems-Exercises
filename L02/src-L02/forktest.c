#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("I'm process %d\n", getpid());

    switch ( fork() ) {
      case -1: perror("fork"); exit(1);	 // error
	  
      case 0: printf("I'm child %d\n", getpid());
              break;
      default: printf("I'm father %d\n", getpid());
               wait(NULL);		// wait for child exit
    }
    printf("Bye!\n");
    return 0;
}
