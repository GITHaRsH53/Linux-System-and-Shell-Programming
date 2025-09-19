#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<malloc.h>
#include<unistd.h>

int main(){

    ssize_t sz;
    off_t seek_pos=0;  // return type of lseek() sys call
    char buf[200];

    int fd =open("xyz",O_RDWR);
    if (fd < 0){
        perror("Error opening file");
        exit(1);
    }

    seek_pos = lseek(fd,5,SEEK_SET); // chamges the pointer location to 6th pos

    sz = read(fd,buf,10);  // now storing it in our buffer memory/array.
    buf[sz]='\0';          // terminating the string

    printf("\n Read bytes are as follows: %s ",buf);

    seek_pos= lseek(fd,0,SEEK_END);  // changes the pointer to the end of the file
    strcpy(buf,"I have added this line using the Lskeek function with SEEK_SET");  
    sz=write(fd,buf,strlen(buf)); // added this line to buffer array.

    close(fd);

    return 0;
}