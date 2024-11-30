#include "common/program_managment.h"
#include "common/memory.h"

int main();

extern char __bss_start[]; // Linker will set this to point to the start of the program
extern char __bss_end[]; // Linker will set this to point to the end of the program

void entry()    // We still need to set up bss as it doesn't exist atm
{
    size_t bss_size = (size_t)__bss_end - (size_t)__bss_start;
    if (vmemmap(&__bss_start, bss_size, VMEMMAP_WRITABILITY) < bss_size)
        exit(-9999);
        
    memclr(&__bss_start, bss_size);

    exit(main());
}