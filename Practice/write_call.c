#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<malloc.h>
#include<unistd.h>

int main(){
    ssize_t sz;
    char buf[100];

    strcpy(buf,"I am written to the file !! YAY !!! \n "); // copy output in char buffer array.

    int fd =open("xyz",O_WRONLY| O_APPEND); // every time we execute line appends in file
    // int fd = open("xyz",O_WRONLY | O_TRUNC); // every time we execute all file data removes and new line comes

    if(fd < 0) {
        perror("There was an error while opening the file");
        exit(1);
    }

    sz = write(fd,buf,strlen(buf));

    printf("The number of bytes written to the file are as: %d\n",sz);

    return 0;
}