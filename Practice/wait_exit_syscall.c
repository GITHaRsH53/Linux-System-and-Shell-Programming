#include<stdio.h>
#include<stdlib.h>
// #include<sys/wait.h>
#include<unistd.h>

int main(){

    pid_t cpid;
    int status = 0;
    cpid = fork();
    if(cpid == -1) exit(-1);

    if(cpid == 0){
        printf("\n The PID of the child process is as: (%d)\n",getpid());
        sleep(25);
        printf("Printing PID once more and then exiting= %d\n", getpid());
        exit(1); // to avoid orphan process - a process who’s parent has died or finished it’s execution. 
                 // So we need to add exit() to child process to stop orphan process to ever exist. 
                 // thus we need wait() sys call in parent process as it has to wait for its child to finish.
    }

    else{
        printf("\n Parent executing before wait()\n");
        cpid = wait(NULL); // parent will wait until child process completes its execution, until then parent process execution is halted till it receive a signal
        //cpid=wait(&status); // checks status of process. we can kill child process anytime and then automatically parent process will run
        //cpid=wairpid(cpid, &status); //waits for a certain process until it finishes then run the parent process.
        printf("\n wait() in parent done\nParent pid=%d\n", getpid());
        printf("\n cpid returned is (%d)\n",cpid);
        //printf("\n status is (%d)\n",status);
    }

    return 0;
}