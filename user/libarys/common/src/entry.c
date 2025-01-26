#include "common/program_managment.h"
#include "common/memory.h"

int main();

extern char __bss_start[]; // Linker will set this to point to the start of the program
extern char __bss_end[]; // Linker will set this to point to the end of the program

void _entry() __attribute__((section(".start")));

void _create_bss();

void _entry()
{
    _create_bss();

    exit(main());
}

void _create_bss()
{
    const size_t bss_size = (size_t)__bss_end - (size_t)__bss_start;

    if (bss_size == 0)
        return;

    if (vmemmap(&__bss_start, bss_size, VMEMMAP_WRITABILITY) < bss_size)
        exit(-9999);
        
    memclr(&__bss_start, bss_size);
}