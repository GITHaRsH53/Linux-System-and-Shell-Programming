#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2]; // File descriptors for the pipe
    pid_t pid1, pid2;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the first child process to execute "who"
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Child process 1: Execute "who"
        // Redirect stdout to the write end of the pipe
        close(pipefd[0]); // Close the read end
        dup2(pipefd[1], STDOUT_FILENO); // Duplicate write end to stdout
        close(pipefd[1]); // Close the original write end

        // Execute "who"
        execlp("who", "who", NULL);
        perror("execlp who"); // If execlp fails
        exit(EXIT_FAILURE);
    }

    // Fork the second child process to execute "wc -l"
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // Child process 2: Execute "wc -l"
        // Redirect stdin to the read end of the pipe
        close(pipefd[1]); // Close the write end
        dup2(pipefd[0], STDIN_FILENO); // Duplicate read end to stdin
        close(pipefd[0]); // Close the original read end

        // Execute "wc -l"
        execlp("wc", "wc", "-l", NULL);
        perror("execlp wc"); // If execlp fails
        exit(EXIT_FAILURE);
    }

    // Parent process
    // Close both ends of the pipe
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}

/*Explanation:
Pipe Creation:

A pipe is created using pipe(pipefd). It has two file descriptors:

pipefd[0]: Read end of the pipe.

pipefd[1]: Write end of the pipe.

First Child Process (who):

The first child process is created using fork().

It closes the read end of the pipe (pipefd[0]) because it only needs to write to the pipe.

It redirects its standard output (STDOUT_FILENO) to the write end of the pipe using dup2(pipefd[1], STDOUT_FILENO).

It executes the who command using execlp("who", "who", NULL).

Second Child Process (wc -l):

The second child process is created using fork().

It closes the write end of the pipe (pipefd[1]) because it only needs to read from the pipe.

It redirects its standard input (STDIN_FILENO) to the read end of the pipe using dup2(pipefd[0], STDIN_FILENO).

It executes the wc -l command using execlp("wc", "wc", "-l", NULL).

Parent Process:

The parent process closes both ends of the pipe because it doesn’t need them.

It waits for both child processes to finish using waitpid().*/