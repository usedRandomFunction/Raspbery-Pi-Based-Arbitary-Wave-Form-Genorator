#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char __end[]; // Linker will set this to point to the end of the program

#define PROGRAM_END_ADDRESS_POINTER (void*) &__end
#define PROGRAM_END_ADDRESS_SIZE_T (size_t)&__end

#define GPU_UNCACHED_BASE	0xC0000000
#define GPU_CACHED_BASE		0x40000000
#define GPU_MEM_BASE	GPU_CACHED_BASE
#define BUS_ADDRESS(addr)	(((*(uint32_t*)&addr) & ~0xC0000000) | GPU_MEM_BASE)

// #define data_sync_barrier()	asm volatile ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
// #define data_mem_barrier() 	asm volatile ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")

// #define peripheral_entry()	data_sync_barrier()
// #define peripheral_exit()	data_mem_barrier()

typedef void (*FREE_PTR)(void*);
typedef void* (*MALLOC_PTR)(size_t);
typedef void* (*MALLOC_ALIGNED_PTR)(size_t, size_t);

void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, size_t size, uint8_t value);

inline void* void_ptr_offset_bytes(void* ptr, int offset);

inline void* void_ptr_offset_bytes(void* ptr, int offset)
{
    return ((uint8_t*)ptr) + offset;
}

#ifdef __cplusplus
}
#endif

#endif