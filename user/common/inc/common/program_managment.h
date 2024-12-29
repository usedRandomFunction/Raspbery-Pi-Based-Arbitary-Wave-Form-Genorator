#ifndef PROGRAM_MANAGMENT_H
#define PROGRAM_MANAGMENT_H

#include <stddef.h>

enum
{
    VMEMMAP_WRITABILITY     =   1 << 0,     /* If set the allocation will be writeable*/
    VMEMMAP_NON_CACHABLE    =   1 << 1,     /* If set the allocation wont be cachable*/
    VMEMMAP_EXECUTABLE      =   1 << 2      /* If set the allocation will be executable*/
};

// Used to set the ABI version used by the app
// @param version The version that we want to use
// @return 0 If sucesss, -1 if failed
int set_abi_version(int version);

// Exits the current program with code i
// @param i Exit code
void exit(int i);

// Used to manage virtual address translation 
// @param ptr The address for the allocaion to goto
// @param size The size of the allocation (to create / modify), or set to zero to delete, a value of -1 will return the current size
// @param flags flags
// @return The size of the allocation in bytes / 0 if failed, or if deleting > 0 on success
size_t vmemmap(void* ptr, size_t size, int flags);

// Used to simultausly exit the current program while simultaiusly starting the given program
// @param new_executable_path Path to the new program
void switch_to(const char* new_executable_path);

#endif