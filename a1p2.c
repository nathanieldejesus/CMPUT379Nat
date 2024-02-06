#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    // most of the code comes from the AI generated png,
    // with a few tweaks to make it work properly
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
        // child process
        // if execlp is successful
        execlp("./myclock", "myclock", "out1", (char *) NULL);

        // If execlp fails, handle the error
        perror("execlp");
        exit(EXIT_FAILURE);
    }
     
    else {
        // parent
        if (argv[1][0] == 'w') {
            // wait till child terminates then send signal
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
            // sleep 2 min
            sleep(120);
        }
        else {
            // wrong input error
            fprintf(stderr, "Invalid option. Use 'w' or 's'.\n");
            return 1;
        }
    }

    return 0;
}
