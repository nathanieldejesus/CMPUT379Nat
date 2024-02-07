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

/*Prints command from an inputted array of characters*/
void printCommand(char input[NPROC][MAXLINE], int num) {
    printf("print_cmd(): [%s]\n", input[num]);
}

static void printTimes(clock_t time, struct tms *tStartStruct, struct tms *tEndStruct) {
    // used ChatGPT to help with formatting the print statements here
    static long clk;
    clk = sysconf(_SC_CLK_TCK);
    
    double real = time / (double)clk;
    double user = (tEndStruct->tms_utime - tStartStruct->tms_utime) / (double)clk; // user process time
    double sys = (tEndStruct->tms_stime - tStartStruct->tms_stime) / (double)clk; // sys process time
    double childUser = (tEndStruct->tms_cutime - tStartStruct->tms_cutime) / (double)clk; // child user time
    double childSys = (tEndStruct->tms_cstime - tStartStruct->tms_cstime) / (double)clk; // child sys time

    printf("real:         %7.2f sec.\n", real);
    printf("user:         %7.2f sec.\n", user);
    printf("sys:          %7.2f sec.\n", sys);
    printf("child user:   %7.2f sec.\n", childUser);
    printf("child sys:    %7.2f sec.\n", childSys);
}

// using a1-split.c code
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

void checkArguments(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s (n|w|a) < inputFile\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argv[1][0] != 'n' && argv[1][0] != 'w' && argv[1][0] != 'a') {
        fprintf(stderr, "Invalid argument. Use (n|w|a)\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    struct tms tStartStruct, tEndStruct;

    // record start time
    clock_t tStart, tEnd;
    tStart = times(&tStartStruct);

    // check command line argument
    checkArguments(argc, argv);

    int i = 0;
    char line[MAXLINE];
    char inputStored[NPROC][MAXLINE];
    char tokenizedLine[MAXNTOKEN][MAXWORD];
    char fieldSep[] = "\n";
    pid_t pidArray[NPROC];
    
    for (i = 0; i < NPROC && fgets(line, sizeof(line), stdin) != NULL; i++) {
        if (line[0] == '#' || line[0] == '\n') {
            // skip comment lines + empty lines
            i--; // this wasn't a command, subtract the added assumption
            continue;
        }

        // from a1p1q2.c
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len - 1] = '\0';
        }

        int tokenCount = splitLine(line, tokenizedLine, fieldSep);
        
        //initialize the pointer array that will hold the commands
        char *tokenizedPtr[MAXNTOKEN + 1];
        tokenizedPtr[tokenCount + 1] = NULL;
        for (int j = 0; j < tokenCount; j++) tokenizedPtr[j] = tokenizedLine[j];
      
        pid_t cpid = fork();

        pidArray[i] = cpid;
        strcpy(inputStored[i], line);
     
        if (cpid == -1) {
            // fork didn't work
            perror("fork");
            exit(EXIT_FAILURE);
        } 
        
        if (cpid == 0) {
            // child process
            printCommand(inputStored, i);
            execvp(tokenizedPtr[0], tokenizedPtr);
            exit(EXIT_SUCCESS);

            // if execvp fails
            perror("execvp");
            exit(EXIT_FAILURE);
        } 

        else {
            char arg = argv[1][0];

            if (arg == 'n') {
                continue;
            }

            else if (arg == 'w') {
            // If 'w' is written, wait for the child process to terminate
                int status;
                // where there 
                if (i < 1) waitpid(cpid, &status, 0);
            }

            else if (arg == 'a') { 
                int status;
                // from a1p2.c  
                if (WIFEXITED(status) && WEXITSTATUS(status) != 1) {
                    printf("process (%d: '%s') terminated (exit code: %d)\n", cpid, line, WEXITSTATUS(status));  
                } 
                else if (WIFSIGNALED(status)) {
                    printf("process terminated (signal: %d)\n", WTERMSIG(status));
                }
                else {
                    printf("(%d) cannot execute '%s'\n", cpid, line);
                }

            }
            tEnd = times(&tEndStruct);
            clock_t out = tEnd - tStart;
            printTimes(out, &tStartStruct, &tEndStruct);
            
        } 
    }

    printf("Process table: \n");
    for (int k = 0; k < NPROC; k++) printf("%d [%d: '%s']\n", k, pidArray[k], inputStored[k]);
    return 0;
}
