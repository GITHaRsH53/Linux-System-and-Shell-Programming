#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork(); // Create a child process

    if (pid > 0) { // Parent process
        printf("Parent process (PID: %d) is running.\n", getpid());
        sleep(2); // Sleep to ensure the child process is still running
        printf("Parent process (PID: %d) is terminating.\n", getpid());
        exit(0); // Parent process terminates
    } else if (pid == 0) { // Child process
        printf("Child process (PID: %d) is running.\n", getpid());
        sleep(5); // Sleep to ensure the parent process terminates first
        printf("Child process (PID: %d) is now an orphan.\n", getpid());
        printf("Child process (PID: %d) is terminating.\n", getpid());
    } else { // Fork failed
        perror("fork");
        exit(1);
    }

    return 0;
}