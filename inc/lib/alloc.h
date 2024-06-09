#ifndef _H
#define _H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define alloca(x) __builtin_alloca((x))
#else
#error failed to find alloca
#endif

// Creates a new allocation on the heap
// @param alignment The required alignment in bytes
// @param size The size of the allocation in bytes
// @return The pointer to the allocation
void* aligned_alloc(size_t alignment, size_t size);

// Creates a new allocation on the heap
// @param size The size of the allocation in bytes
// @return The pointer to the allocation
void* malloc(size_t size);

// Frees the given allocation
// @param p A pointer to the allocation to free
void free(void* p);


#ifdef __cplusplus
}
#endif

#endif