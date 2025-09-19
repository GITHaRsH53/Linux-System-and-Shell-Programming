#include <stdio.h>
#include <unistd.h> // for write syscall
#include <string.h>

int main(){
    size_t length;
    int lenString;
    char buf[100]; // created buffer array
    strncpy(buf,"Hello! I am Printed through the use of a system call.",99); // passed the string to that buffer array
    lenString = strlen(buf);
    length = write(1,buf,lenString);   // write sys call to print it to the terminal
    return 0;
}