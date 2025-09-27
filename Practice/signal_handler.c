#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void signal_handler (int signo){
    if (signo == SIGINT)
    printf ("\n Caught SIGINT!\n");
    else if (signo == SIGTERM)
    printf ("\n Caught SIGTERM!\n");
    exit (EXIT_SUCCESS); // if we not write this then process won't terminate. then seperately we have to sudo kill -9 'pid'
}

int main (void){
    printf("\n process ID is(%d)\n",getpid());
#if 1 // if u write 0 instead of 1, then this if will be commented out
    if (signal (SIGINT, signal_handler) == SIG_ERR) { // will return "caught sigint if ctrl + c"
        fprintf (stderr, "Cannot handle SIGHUP!\n"); 
        exit (-1);
    }

#endif
    //if (signal (SIGINT, SIG_DFL)); // default signal - if we use this then ctrl + c will kill process but no return value -> doesn't go inside signal handler function.
    //if (signal (SIGINT, SIG_IGN)); // ignore signal - ignores 
    if (signal (SIGTERM, signal_handler) == SIG_ERR) {
        fprintf (stderr, "Cannot handle SIGTERM!\n");
        exit (-1);
    }
    while(1);
}