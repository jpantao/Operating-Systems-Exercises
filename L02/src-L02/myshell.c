#include <string.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ARGVMAX 100
#define LINESIZE 1024

int makeargv(char *s, char* argv[ARGVMAX]) {
  // in: s points a text string with words
  // out: argv[] points to all words in the string s (*s is modified!)
  // pre: argv is predefined as char *argv[ARGVMAX]
  // return: number of words pointed to by the elements in argv (or -1 in case of error) 

  int ntokens;
	
  if ( s==NULL || argv==NULL || ARGVMAX==0) 
    return -1;

  ntokens = 0;
  argv[ntokens]=strtok(s, " \t\n");
  while ( (argv[ntokens]!= NULL) && (ntokens<ARGVMAX) ) {
    ntokens++;
    argv[ntokens]=strtok(NULL, " \t\n"); 
  }
  argv[ntokens] = NULL; // it must terminate with NULL
  return ntokens; 
}


void runcommand(char* argv[]) {
   // work assignment
   
   switch( fork() ){
     case -1: perror("fork"); exit(1);

     case 0: printf("should execute: \n");
             for( int i=0; argv[i]!=NULL; i++){
               printf("%s\n", argv[i]);
             }
             exit(0);
     default: wait(NULL); // wait for child exit 
   }

   if ( strcmp( argv[0] ,"exit") == 0){
     exit(0);
   }
    
   
}


int main() {
  char line[LINESIZE];
  char* av[ARGVMAX];
	
  printf("> "); fflush(stdout); //writes the prompt on the standard output 
  while ( fgets( line, LINESIZE, stdin ) != NULL ) {
    if ( makeargv( line, av) > 0 ) 
      runcommand( av );
    printf("> "); fflush(stdout);
  }

  return 0;
}

