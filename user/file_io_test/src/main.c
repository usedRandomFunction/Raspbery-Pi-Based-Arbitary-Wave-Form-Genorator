#include "common/sys_calls.h"
#include "common/memory.h"

void main() 
{
    int fd = open("test.txt", 0);

    if (fd == -1)
        exit(-1);

    char buffer[1024];
    memclr(buffer, 1024);

    read(fd, buffer, 1023);

    printf("test.txt: %s\n", buffer);

    close(fd);

    exit(0);
}