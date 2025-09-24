#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main() {
    pid_t id; // return type of fork
    printf("The ID of the Parent Process is as: %d\n",getpid());
    id= fork(); // now below this code is duplicated one will execute in parent and one in child process.
    if (id < 0 ){
        printf("\nfork process creation has failed failed\n");
        exit(-1);
    }
    if(id >0){
        printf("\n The Parent Process has created a child process with PID as : %d\n",id);
    }
    else{ // inside child virtual memory or process
        printf("\n I am the child process and my id as as: %d\n",id) ;
        printf("\n child process id by using getpid() function is as: (%d)\n",getpid());
        printf("\n The parent of this child is as : (%d) using the getppid() function\n",getppid());
    }
    return 0;
}