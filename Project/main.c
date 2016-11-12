#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "util.h"

#define ON 1
#define OFF 0

/* Helper function: finds specific target from the graph */
struct target* findTarget(struct target *targetArray, char *targetName){
    int n = 10;
    int i;
    for(i = 0; i < n; i++){
        if(strcmp(targetArray[i].szTarget, targetName) == 0){
            return &targetArray[i];
        }
    }
    return NULL;
}

/* Check dependency timestamps for a certain target */
int checkDependencyModTimes(struct target* currentTarget, struct target *targetArray) {
    struct target* temp = findTarget(targetArray, currentTarget->szTarget);
    int i=0;
    while(temp->szDependencies[i] != NULL && strlen(temp->szDependencies[i]) != 0) {
        int val = compare_modification_time(temp->szTarget, temp->szDependencies[i]);
        if (val != 1) {
            return 1;
        }
        i = i + 1;
    }
    if (strlen(temp->szDependencies[0]) == 0) {
        return 1;
    }
    return -1;
}

/* executeTarget: Performs a Recursive Depth-first search on the graph generated from
  the parse function */
void executeTarget(struct target *targetArray, char *targetName, int enable_execute, int enable_timestamps_check){
    int i = 0;
    struct target* currentTarget = findTarget(targetArray, targetName);
    int depCount = currentTarget->nDependencyCount;
    int pid = -1;
    int status;
    
    if (enable_timestamps_check == 1) {
        int check = checkDependencyModTimes(currentTarget, targetArray);
        if (check != 1) {
            printf("%s is up-to date.\n", currentTarget->szTarget);
            currentTarget->nTimeStatus = 1;
            return;
        }
    }
    
    if(depCount < 1 && currentTarget->nTimeStatus != 1) {
        char **myargv;
        int count = makeargv(currentTarget->szCommand, " ", &myargv);
        if (enable_execute == 1) {
            printf("%s \n", currentTarget->szCommand);
            pid = fork();
            if(pid == 0){
                if (execvp(myargv[0], myargv) == -1)
                {
                    printf("Failure to execute because %s\n", strerror(errno));
                    exit(0);
                }
            } 
            else {
                pid = wait(&status);
            }
        }
        else {
            printf("Command: %s \n", currentTarget->szCommand);
        }
        return;
    }else {
        int i;
        for(i = 0; i < depCount; i++){
            char* currentDep = currentTarget->szDependencies[i];
            if(currentDep[strlen(currentDep) - 1] == 'c'){
            }else {
                struct target* newTarget = findTarget(targetArray, currentDep);
                if(newTarget == NULL){
                    printf("NULL: ");
                }else{
                    if(newTarget->nStatus == 0){
                        executeTarget(targetArray, newTarget->szTarget, enable_execute, enable_timestamps_check);
                        newTarget->nStatus = 1;
                    }
                }
            }
            (*currentTarget).nDependencyCount = depCount - 1;
        }
    }
    if(currentTarget->nStatus == 0)
        executeTarget(targetArray, currentTarget->szTarget, enable_execute, enable_timestamps_check);
    
}

/* parse: This function will parse makefile input from user or default makeFile. */
int parse(char * lpszFileName, struct target *targetArray, int index)
{
    int nLine = 0;
    char szLine[1024];
    char * lpszLine;
    FILE * fp = file_open(lpszFileName);
    int onCommand = 0;
    int commandCount = 0;
    
    if(fp == NULL)
    {
        return -1;
    }
    
    // this loop will go through the given file, one line at a time
    while(file_getline(szLine, fp) != NULL)
    {
        nLine++;
        
        //Remove newline character at end if there is one
        lpszLine = strtok(szLine, "\n");
        
        // Now, lpszLine will basically be each line
        if (lpszLine != NULL) {
        
            //Remove leading whitespace.
            while ((*lpszLine) == ' ') lpszLine++;
            
            //Skip if blank or comment. Blank Lines are handled by strtok
            if (lpszLine[0] == '#') {
                commandCount = 0;	// Reset command count;
                continue;
            }
            
            //check to see if we are on command or target line.
            if(strlen(targetArray[index].szTarget) == 0 || (targetArray[index].szTarget[0]) =='\0'){
                onCommand = 0;
            }else{
                onCommand = 1;
            }
        
            // The line with Command: Only single command is allowed.
            if(onCommand == 1){
                if (lpszLine[0] == '\t') {
                    lpszLine++;
                    commandCount++;
                    if (commandCount > 1) {
                        printf("%s\n", "More than 1 command. Parsing failed.");
                        return EXIT_FAILURE;
                    }
                    targetArray[index].nStatus = 0;
                    targetArray[index].nTimeStatus = 0;
                    strcpy(targetArray[index].szCommand, lpszLine);
                    int i;
                    index++;
                } else {
                    printf("Error! Command for target %s does not start with tab.\n", targetArray[index].szTarget);
                    return EXIT_FAILURE;
                }
            }
            else {
                if(lpszLine[0] == ' ' || lpszLine[0] == '\t'){
                    printf("Error! Target line starts with tab or space.\n");
                    return EXIT_FAILURE;
                }
                // Handle the target line here:
                commandCount = 0;	// Reset command count;
                char* token = strtok(lpszLine, ":");
                strcpy(targetArray[index].szTarget, lpszLine);
                token = strtok(NULL, " ");
                int count = 0;
                while(token != NULL){
                    strcpy(targetArray[index].szDependencies[count], token);
                    count++;
                    token = strtok(NULL, " ");
                }
                targetArray[index].nDependencyCount = count;
            }
        }
    }
    
    //Close the makefile.
    fclose(fp);
    return 0;
}

void show_error_message(char * lpszFileName)
{
    fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
    fprintf(stderr, "-f FILE\t\tRead FILE as a maumfile.\n");
    fprintf(stderr, "-h\t\tPrint this message and exit.\n");
    fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
    fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
    fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
    exit(0);
}

int main(int argc, char **argv)
{
    // Declarations for getopt
    extern int optind;
    extern char * optarg;
    int ch;
    char * format = "f:hnBm:";
    
    // Default makefile name will be Makefile
    char szMakefile[64] = "Makefile";
    char szTarget[64];
    
    char szLog[64];
    int log;
	bool redir=false;
    
    // For Command Line Options:
    int enable_execute = ON;
    int enable_timestamps_check = ON;
    
    while((ch = getopt(argc, argv, format)) != -1)
    {
        switch(ch)
        {
            case 'f':
                strcpy(szMakefile, strdup(optarg));
                break;
            case 'n':
                enable_execute = OFF;
                break;
            case 'B':
                enable_timestamps_check = OFF;
                break;
            case 'm':
				redir = true;
				strcpy(szLog, strdup(optarg));
				break;
            case 'h':
            default:
                show_error_message(argv[0]);
                exit(1);
        }
    } 
    
    argc -= optind;
    argv += optind;
    
    // at this point, what is left in argv is the targets that were
    // specified on the command line. argc has the number of them.
    // If getopt is still really confusing,
    // try printing out what's in argv right here, then just running
    // with various command-line arguments.
    
    if(redir == true)
	{
		log = open(szLog, O_CREAT|O_TRUNC|O_WRONLY, 0644);
		dup2(log, 1);
	}
    
    if(argc > 1)
    {
        show_error_message(argv[0]);
        return EXIT_FAILURE;
    }
    
    
    struct target targetArray[10];
    int i;
    for(i = 0; i < 10; i++){
        targetArray[i].szTarget[0] = '\0';
    }
    /* Parse graph file or die */
    if((parse(szMakefile, targetArray, 0)) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    
    //You may start your program by setting the target that make4061 should build.
    //if target is not set, set it to default (first target from makefile)
    if(argc == 1)
    {
        executeTarget(targetArray, argv[0], enable_execute, enable_timestamps_check);
    }
    else
    {
        executeTarget(targetArray, targetArray[0].szTarget, enable_execute, enable_timestamps_check);
    }
    
    //after parsing the file, you'll want to check all dependencies (whether they are available targets or files)
    //then execute all of the targets that were specified on the command line, along with their dependencies, etc.
    return EXIT_SUCCESS;
}
