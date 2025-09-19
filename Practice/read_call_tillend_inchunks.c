#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<malloc.h>
#include<unistd.h>

int main() {
    int fd;
    ssize_t sz;
    char buf[100] = {0};

    fd=open("xyz",O_RDONLY);

    if(fd < 0) {
        perror("Error Opening File");
        exit(1);
    }

    while((sz = read(fd,buf,sizeof(buf) - 1))>0){  // reads till the end.
        buf[sz] = '\0';                            // for termination of string.
        printf(" Chunk read \n %s", buf);
    }
    if(sz < 0) {
        perror("error reading list"); 
        exit(1);
    }

    return 0;
}