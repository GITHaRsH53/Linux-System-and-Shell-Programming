#include<stdio.h>
#include<stdlib.h>
// #include<sys/wait.h> - >in linux env 
#include<unistd.h>

int main (){

    pid_t cpid;
    cpid = fork(); // duplicated, first the parent process execute then child.
    if(cpid ==- 1) exit(-1);

    if(cpid == 0){
        printf("\nChild: Before exec\n");
        execl("./exec2","arg1","arg2",NULL); // below this now content of exec2.c file is overwritten. so that will come as output.
        printf("\n Child: line is not printed\n");
    }
    else printf("I am the Parent Process");

    return 0;
}