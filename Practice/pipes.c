#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(){

    int fd[2]; // 1 - write in pipe, 0 for read in pipe.   
    char buff[50];
    char data[50] = {};

    if (pipe(fd) ==- 1) perror("pipe");
    exit(1);

    sprintf(buff,"PIPE data example");

    write(fd[1], buff, strlen(buff));
    printf("\n");

    read(fd[0], data, 5);
    printf("%s\n", data);

    read(fd[0], data, 5);
    printf("%s\n", data);

    read(fd[0], data, 10); 
   printf("%s\n", data);
}


