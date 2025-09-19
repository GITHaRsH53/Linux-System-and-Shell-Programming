#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <command> <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *command = argv[1];  // Command to execute (e.g., "wc")
    char *input_file = argv[2];  // Input file (e.g., "f1")
    char *output_file = argv[3];  // Output file (e.g., "f2")

    // Open the input file for reading
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("open input file");
        exit(EXIT_FAILURE);
    }

    // Redirect stdin to the input file
    if (dup2(input_fd, STDIN_FILENO) == -1) {
        perror("dup2 stdin");
        close(input_fd);
        exit(EXIT_FAILURE);
    }
    close(input_fd);  // Close the original file descriptor

    // Open the output file for writing (create if it doesn't exist, truncate if it does)
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open output file");
        exit(EXIT_FAILURE);
    }

    // Redirect stdout to the output file
    if (dup2(output_fd, STDOUT_FILENO) == -1) {
        perror("dup2 stdout");
        close(output_fd);
        exit(EXIT_FAILURE);
    }
    close(output_fd);  // Close the original file descriptor

    // Execute the command
    execlp(command, command, (char *)NULL);

    // If execlp returns, it means an error occurred
    perror("execlp");
    exit(EXIT_FAILURE);
}