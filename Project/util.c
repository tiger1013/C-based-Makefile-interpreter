/************************
 * util.c
 *
 * utility functions
 *
 ************************/

#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/***************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *******/
FILE * file_open(char* filename) 
{
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) 
	{
		fprintf(stderr, "make4061: %s: No such file or directory.\n", filename);
		exit(1);
	}

	return fp;
}

//This function will return the line.
char* file_getline(char* buffer, FILE* fp) 
{
	buffer = fgets(buffer, 1024, fp);
	return buffer;
}

//Return -1 if file does not exist
int is_file_exist(char * lpszFileName)
{
	return access(lpszFileName, F_OK); 
}


//return -1 if file does not exist. 
//return last modified time of file
//bigger number means that it is newer (more recently modified). 
int get_file_modification_time(char * lpszFileName)
{
	if(is_file_exist(lpszFileName) != -1)
	{
		struct stat buf;
		int nStat = stat(lpszFileName, &buf);
		return buf.st_mtime;
	}
	
	return -1;
}

//Compare the last modified time between two files.
//return -1, if any one of file does not exist. 
//return 0, if both modified time is the same.
//return 1, if first parameter is bigger (more recent)
//return 2, if second parameter is bigger (more recent)
int compare_modification_time(char * lpsz1, char * lpsz2)
{	
	int nTime1 = get_file_modification_time(lpsz1);
	int nTime2 = get_file_modification_time(lpsz2);

	if(nTime1 == -1 || nTime2 == -1)
	{
		return -1;
	}

	if(nTime1 == nTime2)
	{
		return 0;
	}
	else if(nTime1 > nTime2)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

// makeargv
/* Taken from Unix Systems Programming, Robbins & Robbins, p37 */
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }

   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

//You should call this function when you done with makeargv()
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}
//ALL functions in the following are custom-made functions


void makeRegex (regex_t *re, const char* pattern) {
  int rc ;

  /* "Compile" the regular expression.  This sets up the regex to do
     the matching specified by the regular expression given as a
     character string.
   */
  rc = regcomp(re, pattern, REG_EXTENDED) ;

  if (rc!= 0) {
      printf ("Error in compiling regular expression.\n");
      size_t length = regerror (rc, re, NULL, 0) ;
      char *buffer = (char *) malloc( sizeof(char) * length ) ;
      (void) regerror (rc, re, buffer, length) ;
      printf ("%s\n", buffer);
  }
}


bool matchRegex (regex_t *re, const char *text) {
    int status ;
    const int nsub=1 ;
    regmatch_t matches[nsub] ;

  /* execute the regular expression match against the text.  If it
     matches, the beginning and ending of the matched text are stored
     in the first element of the matches array.
   */
    status = regexec(re, text, (size_t)nsub, matches, 0); 

    if (status==REG_NOMATCH) {
        return false ;
    }
    else {
        return true ;
    }
}
