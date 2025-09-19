#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main() {
    int client_to_server_pipe[2]; // Pipe for client to server communication
    int server_to_client_pipe[2]; // Pipe for server to client communication
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int fd, bytes_read;

    // Create the pipes
    if (pipe(client_to_server_pipe) == -1 || pipe(server_to_client_pipe) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork to create the client process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process (Client)
        close(client_to_server_pipe[0]); // Close read end of client-to-server pipe
        close(server_to_client_pipe[1]); // Close write end of server-to-client pipe

        // Get the filename from the user
        printf("Client: Enter the filename: ");
        fgets(filename, BUFFER_SIZE, stdin);
        filename[strcspn(filename, "\n")] = '\0'; // Remove newline character

        // Send the filename to the server
        write(client_to_server_pipe[1], filename, strlen(filename) + 1);
        close(client_to_server_pipe[1]); // Close write end after sending

        // Read the file contents from the server
        while ((bytes_read = read(server_to_client_pipe[0], buffer, BUFFER_SIZE)) > 0) {
            write(STDOUT_FILENO, buffer, bytes_read); // Display the contents on the monitor
        }
        close(server_to_client_pipe[0]); // Close read end after reading
    } else {
        // Parent process (Server)
        close(client_to_server_pipe[1]); // Close write end of client-to-server pipe
        close(server_to_client_pipe[0]); // Close read end of server-to-client pipe

        // Read the filename from the client
        read(client_to_server_pipe[0], filename, BUFFER_SIZE);
        close(client_to_server_pipe[0]); // Close read end after reading

        // Open the file
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            perror("open");
            write(server_to_client_pipe[1], "Error: File not found or cannot be opened.\n", 44);
            close(server_to_client_pipe[1]);
            exit(EXIT_FAILURE);
        }

        // Read the file contents and send them to the client
        while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
            write(server_to_client_pipe[1], buffer, bytes_read);
        }
        close(fd); // Close the file
        close(server_to_client_pipe[1]); // Close write end after sending
    }

    return 0;
}