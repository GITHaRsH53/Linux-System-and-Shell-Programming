#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // The first argument is the command to execute
    char *command = argv[1];

    // The rest of the arguments are passed to the command
    char **args = &argv[1];

    // Execute the command
    execvp(command, args);

    // If execvp returns, it means an error occurred
    perror("execvp");
    exit(EXIT_FAILURE);
}