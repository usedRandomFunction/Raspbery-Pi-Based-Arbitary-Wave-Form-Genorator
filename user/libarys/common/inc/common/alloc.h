#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

#ifdef __GNUC__
#define alloca(x) __builtin_alloca((x))
#else
#error failed to find alloca
#endif

// Creates a new allocation on the heap
// @param alignment The required alignment in bytes
// @param size The size of the allocation in bytes
// @return The pointer to the allocation
// @note While this header file is part of common, they function deffintions are not
void* aligned_alloc(size_t alignment, size_t size);

// Creates a new allocation on the heap
// @param size The size of the allocation in bytes
// @return The pointer to the allocation
// @note While this header file is part of common, they function deffintions are not
void* malloc(size_t size);

// Frees the given allocation
// @param p A pointer to the allocation to free
// @note While this header file is part of common, they function deffintions are not
void free(void* p);

#endif