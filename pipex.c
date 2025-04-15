#include <stdio.h>      // For input/output functions like perror()
#include <stdlib.h>     // For exit() and EXIT_FAILURE
#include <unistd.h>     // For fork(), pipe(), dup2(), execvp()
#include <sys/wait.h>   // For wait()
#include <string.h>     // For strtok() used in splitting command strings
#include <errno.h>      // For errno, though not directly used here

// A helper function to print error messages and exit the program.
void error_exit(const char *msg) {
    perror(msg);                // Print a description of the error that occurred
    exit(EXIT_FAILURE);         // Exit with a failure status code
}

int main(int argc, char *argv[]) {
    // Check if there are enough arguments (at least one command to run)
    if (argc < 2) {
        fprintf(stderr, "Usage: %s cmd1 [cmd2 ... cmdN]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Number of commands provided as arguments
    int num_cmds = argc - 1;
    int in_fd = STDIN_FILENO;   // Start with standard input (what we normally type)
    int pipefd[2];              // Array to hold the two ends of a pipe

    // Loop over every command given
    for (int i = 0; i < num_cmds; i++) {
        // Create a pipe for all commands except the last one
        if (i < num_cmds - 1) {
            if (pipe(pipefd) == -1)
                error_exit("pipe");
        }

        // Create a new process for the command using fork()
        pid_t pid = fork();
        if (pid < 0) {
            error_exit("fork");
        } else if (pid == 0) {
            // In the child process:
            // If this is not the first command, set the standard input (STDIN)
            // to the previous pipe's read end (in_fd)
            if (i > 0) {
                if (dup2(in_fd, STDIN_FILENO) == -1)
                    error_exit("dup2 in_fd");
            }
            // If this is not the last command, set the standard output (STDOUT)
            // to the current pipe's write end, so output goes into the pipe
            if (i < num_cmds - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                    error_exit("dup2 pipefd[1]");
            }

            // Close the unnecessary file descriptors in the child process
            if (i > 0)
                close(in_fd);
            if (i < num_cmds - 1) {
                close(pipefd[0]);  // Child doesn't need the read end of the current pipe
                close(pipefd[1]);  // And we've already duplicated this to STDOUT
            }
            
            // Prepare the command and its arguments.
            // This tokenizes the command string (e.g., "ls -l") into an array of strings.
            char *args[64];
            int arg_index = 0;
            char *token = strtok(argv[i + 1], " ");
            while (token != NULL && arg_index < 63) {
                args[arg_index++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_index] = NULL; // The argument list must be NULL-terminated
            
            // Replace the child process with the new command.
            // execvp searches for the command in the system's PATH.
            execvp(args[0], args);
            // If execvp returns, it means an error occurred.
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // In the parent process:
            // Close the previous pipe's read end (if any) because it's no longer needed.
            if (i > 0)
                close(in_fd);
            // For all commands except the last, set in_fd to the current pipe's read end.
            if (i < num_cmds - 1) {
                close(pipefd[1]);  // Parent doesn't write into the current pipe
                in_fd = pipefd[0];
            }
        }
    }
    
    // Wait for all the child processes to finish.
    for (int i = 0; i < num_cmds; i++)
        wait(NULL);

    return 0;
}
