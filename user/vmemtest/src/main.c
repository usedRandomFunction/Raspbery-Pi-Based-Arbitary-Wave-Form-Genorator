#include "common/sys_calls.h"
#include "common/memory.h"

void main() 
{
    int fd = open("test.txt", 0);

    if (fd == -1)
        exit(-1);

    char* buffer = (char*)0x000080000000;

    size_t n = vmemmap(buffer, 1024, VMEMMAP_WRITABILITY);

    if (n == 0)
        exit(-2);

    n = read(fd, buffer, n);

    if (n == -1)
        exit(-3);

    if (n == 0)
        n = get_file_size(fd);

    buffer[n] = '\0';

    printf("test.txt: %s\n", buffer);

    close(fd);

    exit(0);
}