#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char _start[]; // Linker will set this to point to the start of the program
extern char __end[]; // Linker will set this to point to the end of the program

#define PROGRAM_START_ADDRESS_POINTER (void*) &_start
#define PROGRAM_START_ADDRESS_SIZE_T (size_t)&_start

#define PROGRAM_END_ADDRESS_POINTER (void*) &__end
#define PROGRAM_END_ADDRESS_SIZE_T (size_t)&__end

#ifdef AARCH64
#define data_sync_barrier() asm volatile ("dsb sy" : : : "memory")
#define data_mem_barrier()  asm volatile ("dmb sy" : : : "memory")
#else
#define data_sync_barrier()	asm volatile ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#define data_mem_barrier() 	asm volatile ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#endif

#define peripheral_entry()	data_sync_barrier()
#define peripheral_exit()	data_mem_barrier()

typedef void (*FREE_PTR)(void*);
typedef void* (*MALLOC_PTR)(size_t);
typedef void* (*MALLOC_ALIGNED_PTR)(size_t, size_t);

// Copys n bytes from src to dst
// @param dst The destination buffer
// @param src The soruce buffer
// @param n The number of bytes to copy
void* memcpy(void* dst, const void* src, size_t n);

// Sets n bytes of dst to value
// @param dst The destination buffer
// @param size The number of bytes to set
// @param value The value to write
// @return a pointer to the destination buffer
void* memset(void* dst, size_t size, uint8_t value);


// Sets n bytes of dst to 0
// @param dst The destination buffer
// @param size The number of bytes to set
// @return a pointer to the destination buffer
void* memclr(void* dst, size_t size);

// Adds a offest to the ptr as if it were uint8_t*
// @param ptr The pointer to offset
// @param offset The number of bytes to add (can be negitive)
// @return The pointer with the offset
inline void* void_ptr_offset_bytes(void* ptr, ptrdiff_t offset);

// Peformce a bitwise and on the pointer as if it is a number
// @param ptr The pointer modify 
// @param mask The mask to bitwise and with
// @return The modifyed pointer
inline void* void_ptr_bitwise_and(void* ptr, size_t mask);

// Uses the tranlation table to the physical address of a pointer
// @param virtual_ptr Pointer to translate
// @return The translated pointer
inline void* get_physical_address(void* virtual_ptr);

inline void* void_ptr_offset_bytes(void* ptr, ptrdiff_t offset)
{
    return ((uint8_t*)ptr) + offset;
}

inline void* void_ptr_bitwise_and(void* ptr, size_t mask)
{
    size_t new_ptr = *((size_t*)&ptr) & mask;
    return  *((void**)&new_ptr);
}

inline void* get_physical_address(void* virtual_ptr)
{
    unsigned long par_el1;
    asm volatile (
        "at S1E1R, %1\n"
        "mrs %0, par_el1\n"
        : "=r" (par_el1)
        : "r" (virtual_ptr)
        : "memory");
    
    // Check for translation faults in PAR_EL1
    if (par_el1 & 0x1) {
        // If there was a translation fault, return NULL or handle the fault
        return NULL;
    }

    // Extract the physical address from PAR_EL1
    return (void*)((par_el1 & 0xFFFFFFFFF000) + ((*(size_t*)&virtual_ptr) & 0xFFF));
}
#ifdef __cplusplus
}
#endif

#endif