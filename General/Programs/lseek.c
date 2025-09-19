#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<stdio.h>
int main()
{
    int n, f;
    char buff[10];
    f = open("seeking.txt", O_RDWR);
    read(f, buff, 10);
    write(1, buff, 10);
    printf("\n");
    lseek(f, 5, SEEK_SET); // skips 5 characters from the current position
    read(f, buff, 10);
    write(1, buff, 10);
    // printf("\n");
    // lseek(f, 5, SEEK_CUR); // skips 5 characters from the current position
    // read(f, buff, 10);
    // write(1, buff, 10);
    // printf("\n");
    // lseek(f, 5, SEEK_END); // skips 5 characters from the current position
    // read(f, buff, 10);
    // write(1, buff, 10);
}