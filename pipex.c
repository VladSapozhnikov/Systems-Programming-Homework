// pipex.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s cmd1 [cmd2 ... cmdN]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_cmds = argc - 1;
    int in_fd = STDIN_FILENO;
    int pipefd[2];

    for (int i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(pipefd) == -1)
                error_exit("pipe");
        }

        pid_t pid = fork();
        if (pid < 0) {
            error_exit("fork");
        } else if (pid == 0) {
            if (i > 0) {
                if (dup2(in_fd, STDIN_FILENO) == -1)
                    error_exit("dup2 in_fd");
            }
            if (i < num_cmds - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                    error_exit("dup2 pipefd[1]");
            }
            if (i > 0)
                close(in_fd);
            if (i < num_cmds - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            
            // Tokenize the command string into arguments
            char *args[64];
            int arg_index = 0;
            char *token = strtok(argv[i + 1], " ");
            while (token != NULL && arg_index < 63) {
                args[arg_index++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_index] = NULL;
            
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            if (i > 0)
                close(in_fd);
            if (i < num_cmds - 1) {
                close(pipefd[1]);
                in_fd = pipefd[0];
            }
        }
    }
    
    for (int i = 0; i < num_cmds; i++)
        wait(NULL);

    return 0;
}
