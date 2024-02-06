#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 128 // Max # of characters in an input line
#define MAXNTOKEN 16 // Max # of tokens in any input line
#define MAXWORD 20 // Max # of characters in any token
#define NPROC 5 // Max # of commands in a test file

int split(char inStr[],  char token[][MAXWORD], char fs[])
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

                strcpy (inStr, inStrCopy);              // restore inStr
                return(count);
}


static void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend) {
        static long clktck = 0;
        if (clktck == 0) /* fetch clock ticks per second first time */
        clktck = sysconf(_SC_CLK_TCK);

        printf(" real: %7.2f\n", real / (double) clktck);
        printf(" user: %7.2f\n", (tmsend->tms_utime - tmsstart->tms_utime) / (double) clktck);
        printf(" sys: %7.2f\n",(tmsend->tms_stime - tmsstart->tms_stime) / (double) clktck);
        printf(" child user: %7.2f\n",(tmsend->tms_cutime - tmsstart->tms_cutime) / (double) clktck);
        printf(" child sys: %7.2f\n", (tmsend->tms_cstime - tmsstart->tms_cstime) / (double) clktck);
}

static void printTable(int *pidArray[],char lineArray[NPROC][MAXLINE]) {
        // int arraySize = sizeof(pidArray);

        for (int j = 0; j < NPROC; j++) {
                //printf("%d [%d:'%s']\n", j, pidArray[j], lineArray[j]);
        }
}

int main(int argc, char *argv[]) {

        struct tms tmsstart, tmsend;
        clock_t start, end;
        start = times(&tmsstart);

        if(argc != 2) {
                fprintf(stderr, "Usage: %s (n|w|a) <inputFile>\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        int i = 0;
        pid_t pidArray[NPROC];

        char commands[NPROC][MAXLINE];
        char line[MAXLINE];
        char lineArray[NPROC][MAXLINE];
        char token[MAXNTOKEN][MAXWORD];
        char fs[] = " \n";

        while (fgets(line, sizeof(line), stdin) != NULL) {
                if (i == 5) {
                        break;
                }
                if (line[0] == '#' || line[0] == '\n') {
                        continue;
                }

                size_t length = strlen(line); //Part 1 question 2
                if (length > 0 && line[length-1] == '\n') {
                       line[length-1] = '\0';
                }

                int numTokStored = split(line, token, fs);

                char *tokenParameters[MAXNTOKEN+1];
                for (int k = 0; k < numTokStored; k++) tokenParameters[k] = token[k];
                tokenParameters[numTokStored+1] = NULL;
                pid_t childPid = fork();
                pidArray[i] = childPid;
                strcpy(lineArray[i], line);
                if (childPid == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                }

                if (childPid == 0) {
                        printf("print_cmd():[%s]\n", line);
                        execvp(tokenParameters[0], tokenParameters);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                }

                else {
                        if (argv[1][0] == 'w') {
                                int status;
                                if( i < 1) {
                                waitpid(childPid, &status, 0);
                                }
                        }

                        else if (argv[1][0] == 'a') {
                                int status;
                                waitpid(childPid, &status, 0);

                                if (WIFEXITED(status) && WEXITSTATUS(status)!= 1) {
                                        printf("Process (%d:'%s') exited (status = %d)\n", childPid, line,  WEXITSTATUS(status));
                                }

                                else if (WIFSIGNALED(status)) {
                                        fprintf(stderr, "Child process terminated by signal: %d\n", WTERMSIG(status));
                                }

                                else {
                                        printf("Child: (%d): unable to execute '%s'\n", childPid, line);
                                }

                        }

                        else if (argv[1][0] == 'n') {
                                }

                        else {
                                fprintf(stderr, "Invalid option. Use 'w' or 'a' or 'n'.\n");
                                return 1;
                        }
                        end = times(&tmsend);
                        pr_times(end-start, &tmsstart, &tmsend);
                }
                i++;
        }

        //printTable(pidArray,lineArray);


        return  0;
}
