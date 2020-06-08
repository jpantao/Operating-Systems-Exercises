#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <pwd.h>

#define MAXC 2048





void syserror(char *err){
	perror(err);
	exit(1);
}


void myls(char *dir){
	DIR *dp;
	struct dirent *d;
	struct stat inf;
	struct tm timeinf;
  	struct passwd *userinf;

  	if( (dp = opendir(dir)) == NULL) syserror(dir);
	
  	printf("%s:\n", dir);
  	while( (d = readdir(dp)) != NULL ) {
    		char fullname[MAXC];

    		strcat(strcat(strcpy(fullname, dir), "/" ), d-> d_name);
    		if( stat(fullname, &inf) != -1){

   			gmtime_r(&inf.st_mtime, &timeinf); //atualize timeinf to contain the time information

			userinf = getpwuid(inf.st_uid);   //userinf now points to a stucture wiht the info of the owner of the file/dir

      			if( S_ISREG(inf.st_mode) ){
        			printf("%lu: %s %d:%d %s (%ld)\n", inf.st_ino, userinf->pw_name, timeinf.tm_hour, timeinf.tm_min, d->d_name, inf.st_size);
      			}else if( S_ISDIR(inf.st_mode) ){
        			printf("%lu: %s %d:%d %s (dir)\n", inf.st_ino, userinf->pw_name, timeinf.tm_hour, timeinf.tm_min, d->d_name);

      			}else{
        			printf("%lu: %s %d:%d %s (other)\n", inf.st_ino, userinf->pw_name, timeinf.tm_hour, timeinf.tm_min, d->d_name);
      			}

    		}
  	}
  	closedir(dp);
}


void subdir_ls(char *dir){
	DIR *dp;
  	struct dirent *d;
  	struct stat inf;
  	struct tm timeinf;
  	struct passwd *userinf;

  	if( (dp = opendir(dir)) == NULL) syserror(dir);

  	while( (d = readdir(dp)) != NULL ) {
    		char fullname[MAXC];
    		strcat(strcat(strcpy(fullname, dir), "/" ), d-> d_name);
    		if( stat(fullname, &inf) != -1 && S_ISDIR(inf.st_mode) && strcmp(d-> d_name,".") != 0 && strcmp(d-> d_name,"..") != 0 ){
      			putchar('\n');
     	 		myls(fullname);
    		}
  	}
	closedir(dp);
}


int main(int argc, char* argv[]){
  	char *dir;
  	if(argc == 1) dir = ".";
  	else if (argc == 2) dir = argv[1];
  	else {
    		fprintf(stderr, "usage: myls [dir]\n");
    		return 1;
  	}

  	myls(dir);
  	subdir_ls(dir);
  	return 0;

}
