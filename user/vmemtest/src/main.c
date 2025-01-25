#include "common/sys_calls.h"
#include "common/memory.h"

void main() 
{
    printf("Opening test file\n");
    int fd = open("config/outputs.cfg", 0);

    if (fd == -1)
        exit(-1);

    printf("Testing vmemmap (given PTR)\n");

    char* buffer = (char*)0x000080000000;

    size_t n = vmemmap(buffer, 8192, VMEMMAP_WRITABILITY);

    if (n == 0)
        exit(-2);

    n = read(fd, buffer, n);

    if (n == -1)
        exit(-3);

    if (n == 0)
        n = get_file_size(fd);

    buffer[n] = '\0';

    printf("config/outputs.cfg: %s\n", buffer);

    close(fd);

    printf("Opening test file\n");
    fd = open("config/system.cfg", 0);

    if (fd == -1)
        exit(-4);

    printf("Testing vmemmap (find PTR)\n");
    n = vmemmap((void*)&buffer, 8192, VMEMMAP_WRITABILITY | VMEMMAP_RETURN_POINTER);

    if (n == 0)
        exit(-5);

    printf("Got: 0x%x\n", buffer);

    n = read(fd, buffer, n);

    if (n == -1)
        exit(-6);

    if (n == 0)
        n = get_file_size(fd);

    buffer[n] = '\0';

    printf("config/system.cfg: %s\n", buffer);


    exit(0);
}