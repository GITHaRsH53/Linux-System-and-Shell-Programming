#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork(); // Create a child process

    if (pid > 0) { // Parent process
        printf("Parent process (PID: %d) is running.\n", getpid());
        sleep(10); // Sleep to keep the parent process alive
        printf("Parent process (PID: %d) is terminating.\n", getpid());
    } else if (pid == 0) { // Child process
        printf("Child process (PID: %d) is running.\n", getpid());
        printf("Child process (PID: %d) is terminating.\n", getpid());
        exit(0); // Child process terminates
    } else { // Fork failed
        perror("fork");
        exit(1);
    }

    return 0;
}

//ps aux | grep Z