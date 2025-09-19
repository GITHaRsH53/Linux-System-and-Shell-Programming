#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 
  char *input_file = argv[1];
  char *output_file = argv[2];
 
  // Open input file
  int input_fd = open(input_file, O_RDONLY);
  if (input_fd == -1) {
    perror("open input file");
    exit(EXIT_FAILURE);
  }
 
  // Open output file
  int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (output_fd == -1) {
    perror("open output file");
    close(input_fd);
    exit(EXIT_FAILURE);
  }
 
  char buffer[4096];
  ssize_t bytes_read;
  int chars = 0, letters = 0, numbers = 0;
 
  // Read and count
  while ((bytes_read = read(input_fd, buffer, sizeof(buffer))) > 0) {
    chars += bytes_read;
    for (ssize_t i = 0; i < bytes_read; i++) {
      if (isalpha(buffer[i]))
        letters++;
      if (isdigit(buffer[i]))
        numbers++;
    }
  }
 
  close(input_fd);
  // eyes
  //  Format output string
  char output[100];
  int len = snprintf(output, sizeof(output),
                     "Characters: %d\nLetters: %d\nNumbers: %d\n", chars,
                     letters, numbers);
 
  // Write to output file
  if (write(output_fd, output, len) != len) {
    perror("write output");
    close(output_fd);
    exit(EXIT_FAILURE);
  }
 
  close(output_fd);
  return 0;
}