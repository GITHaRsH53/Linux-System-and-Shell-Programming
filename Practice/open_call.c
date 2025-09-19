#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
int main() {

    int fd;

    fd=open("syscall.c",O_RDONLY); // O_RDONLY - is open() sys call function flag for only reading file.
    if (fd == -1) printf("NO SUCH FILE");
    else printf("FD opened file");

    return 0;
}

