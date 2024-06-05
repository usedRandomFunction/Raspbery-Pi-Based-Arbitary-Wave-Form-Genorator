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


typedef void (*FREE_PTR)(void*);
typedef void* (*MALLOC_PTR)(size_t);
typedef void* (*MALLOC_ALIGNED_PTR)(size_t, size_t);

void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, size_t size, uint8_t value);

inline void* void_ptr_offset_bytes(void* ptr, int offset);

inline void* void_ptr_bitwise_and(void* ptr, size_t mask);

inline void* void_ptr_offset_bytes(void* ptr, int offset)
{
    return ((uint8_t*)ptr) + offset;
}

inline void* void_ptr_bitwise_and(void* ptr, size_t mask)
{
    size_t new_ptr = *((size_t*)&ptr) & mask;
    return  *((void**)&new_ptr);
}

#ifdef __cplusplus
}
#endif

#endif