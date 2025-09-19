#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
int main() {

    int fd;

    fd=open("xyz",O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);  // S_IRUSR -> read permission for user
    if (fd == -1) printf("NO SUCH FILE");
    else printf("The newly created also opened");

    return 0;
}

