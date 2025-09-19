#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifndef SIGUSR1
#define SIGUSR1 10 // Define SIGUSR1 if not defined (value may vary by system)
#endif
#include <unistd.h>

// Signal handler function for SIGINT
void sigint_handler(int signum) {
    printf("Caught SIGINT (Signal %d)\n", signum);
}

// Signal handler function for SIGTERM
void sigterm_handler(int signum) {
    printf("Caught SIGTERM (Signal %d)\n", signum);
}

// Signal handler function for SIGUSR1
/*void sigusr1_handler(int signum) {
    printf("Caught SIGUSR1 (Signal %d)\n", signum);
}*/

int main() {
    // Register signal handlers
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Unable to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTERM, sigterm_handler) == SIG_ERR) {
        perror("Unable to set SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    /*if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
        perror("Unable to set SIGUSR1 handler");
        exit(EXIT_FAILURE);
    }*/

    printf("Signal handlers registered. PID = %d\n", getpid());
    printf("Send signals to this process (e.g., SIGINT with Ctrl+C).\n");

    // Keep the program running to catch signals
    while (1) {
        sleep(1);
    }

    return 0;
}

/*Example Output:
Run the program:

bash
Copy
./signal_handler
Press Ctrl+C to send SIGINT:

Copy
Signal handlers registered. Waiting for signals...
^CCaught SIGINT (Signal 2)
In another terminal, send SIGTERM using the kill command:

bash
Copy
kill -TERM <pid>
Output:

Copy
Caught SIGTERM (Signal 15)
Send SIGUSR1 using the kill command:

bash
Copy
kill -USR1 <pid>
Output:

Copy
Caught SIGUSR1 (Signal 10)*/
