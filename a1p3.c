#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <unistd.h>
#include <time.h>

#define MAXLINE 128
#define MAXNTOKEN 16
#define MAXWORD 20
#define NPROC 5

void printCommand(const char *command) {
    printf("print_cmd(): [%s]\n", command);
}

int splitLine(char inStr[],  char token[][MAXWORD], char fs[])
{
    int    i, count;
    char   *tokenp, inStrCopy[MAXLINE]; 

    // initialize variables
    count= 0;
    memset (inStrCopy, 0, sizeof(inStrCopy));
    for (i=0; i < MAXNTOKEN; i++) memset (token[i], 0, sizeof(token[i]));

    // make a backup of inStr in inStrCopy
    strcpy (inStrCopy, inStr);

    if ( (tokenp= strtok(inStr, fs)) == NULL) return(0);
    
    strcpy(token[count],tokenp); count++;

    while ( (tokenp= strtok(NULL, fs)) != NULL)
    {
        strcpy(token[count],tokenp); count++;
    }

     strcpy (inStr, inStrCopy);		// restore inStr
     return(count);	   
}

int main(int argc, char *argv[]) {

    struct tms timesStart, timesEnd;

    // Record start time
    clock_t start_time = clock();
    times(&timesStart);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s (n|w|a)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int i = 0;
    char arg = argv[1][0];
    char line[MAXLINE];
    char tokenizedLine[MAXNTOKEN][MAXWORD];
    char fs[] = "\n";
    pid_t pidArray[NPROC];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (i == 5) {
            break;
        }
        if (line[0] == '#' || line[0] == '\n') {
            // Skip comment lines and empty lines
            continue;
        }

        size_t length = strlen(line);
        //printf("Length before removal: %lu\n", length);
        if (length > 0 && line[length-1] == '\n') {
            line[length - 1] = '\0';
        }

        int count = splitLine(line, tokenizedLine, fs);
        //printCommand(line);
        char *tokenizedPtr[MAXNTOKEN + 1];
        for (int k = 0; k < count; k++) tokenizedPtr[k] = tokenizedLine[k];
        tokenizedPtr[count + 1] = NULL;

        pid_t cpid = fork();

        pidArray[i] = cpid;

        if (cpid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } 
        
        if (cpid == 0) {
            // Child process
            printCommand(line);
            execvp(tokenizedPtr[0], tokenizedPtr);
            //exit(EXIT_SUCCESS);
            // If execvp fails
            perror("execvp");
            exit(EXIT_FAILURE);
        } 

        else {
            if (arg == 'w') {
            // If 'w' is written, wait for the child process to terminate
                int status;
                printf("I am the parent");
                waitpid(cpid, &status, 0);
                
                if (WIFEXITED(status)) {
                    printf("Child process terminated with exit code: %d\n", WEXITSTATUS(status));  
                } else if (WIFSIGNALED(status)) {
                    printf("Child process terminated by signal: %d\n", WTERMSIG(status));
                }

                }
            } 
            i++;
        }


    

    // Record end time
    clock_t end_time = clock();

    // Print recorded times
    printf("real: %.2f sec.\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}
