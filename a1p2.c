#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s (w|s)\n", argv[0]);
        return 1;
    }

    // fork process
    pid_t childPid = fork();

    if (childPid == -1) {
        // handle fork failure (0 is not returned to child)
        perror("fork");
        return 1;
    }
    
    if (childPid == 0) {
        // child 
        
        // if execlp is successful
        execlp("./myclock", "myclock", "out1", (char *) NULL);
        exit(EXIT_SUCCESS);

        // If execlp fails, handle the error
        perror("execlp");
        exit(EXIT_FAILURE);
    }
     
    else {
        // parent
        if (argv[1][0] == 'w') {
            // If 'w' is written, wait for the child process to terminate
            int status;
            waitpid(childPid, &status, 0);
            if (WIFEXITED(status)) {
                printf ("Child process terminated with exit code: %dn", WEXITSTATUS (status));  
            }
            else if (WIFSIGNALED(status)) {
                fprintf(stderr, "Child process terminated by signal: %din", WTERMSIG (status));
            }
        } 
        else if (argv[1][0] == 's') {
            // If 's' is written, sleep for 2 minutes then exit
            sleep(120);
        }
        else {
            // where w|s is not inputted
            fprintf(stderr, "Invalid option. Use 'w' or 's'.\n");
            return 1;
        }
    }

    return 0;
}