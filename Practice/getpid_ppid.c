#include <stdio.h>
#include <unistd.h>   //for getpid() for Linux
#include <sys/types.h>

int main () {

    printf("\nProcess id =%d",getpid()); // changes everytime u execute
    printf("\n Parent Process id = %d",getppid()); // reamins the same as its the id of the shell/terminal
    sleep(7);

}

// will only in Linux enviroment