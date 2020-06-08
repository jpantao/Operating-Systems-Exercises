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


void waitloop (int pid){
  int status;
  int p;

  while( (p = wait(&status)) != pid && p != -1){
    printf("Process %d has concluded its execution with status %d\n", p, WEXITSTATUS(status));
  }
}



void runcommand(char* argv[], int argc) {
  // work assignment
  
  static int backGroundPID;

  int background = 0; 
  int pid;
  int status;

  if ( strcmp( argv[0] ,"exit") == 0){
    exit(0);
  } 

  if ( strcmp( argv[argc-1] ,"&") == 0){
    argv[argc-1] = NULL;
    background = 1;
  }

  if ( strcmp( argv[0] , "fg") == 0){
    if ( argv[1] != NULL){
      backGroundPID = atoi(argv[1]);
    }
    waitpid(backGroundPID, &status, 0);
    printf("Process %d has concluded its execution with status %d\n", backGroundPID, WEXITSTATUS(status));
    
    return;
  }

  switch( pid = fork() ){
    case -1: 
      perror("fork"); exit(1);
    case 0:
      
      execvp(argv[0], argv);
      perror(argv[0]); 
      exit(1);
    default: 
      if (background){
        printf ("Process %d is executing in the background.\n", pid);
        backGroundPID = pid;
      }else{
        waitloop(pid); // wait for child exit 
      }
  }
   
}

void runOneCommand(char *cmd){
   char* av[ARGVMAX];
   int ac;
   if ( (ac =  makeargv( cmd, av)) > 0 ){ 
      runcommand( av, ac);
    }
}


int main() {
  char line[LINESIZE];
  
	
  printf("> "); 
  fflush(stdout); //writes the prompt on the standard output 
  while ( fgets( line, LINESIZE, stdin ) != NULL ) {
    char *start = line;
    char * end;
    
    do{
      end = strchr(start, ';');
      if ( end != NULL){
        *end = '\0';
      }
      runOneCommand(start);
      if(end != NULL){
        start = end + 1;
      }
    }while(end != NULL );
    
  
    printf("> "); 
    fflush(stdout);
  }

  return 0;
}

