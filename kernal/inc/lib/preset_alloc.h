#ifndef PRESET_ALLOC_H
#define PRESET_ALLOC_H

#include <stddef.h>

// Since the property tag functions use alloc these functions exist just to
// Let the property tags be used before the heap is made



// Used to set the two buffers
// The idea is that alloca is used, in the parrent function and 
// this sets up the buffersthe inner functions use this
// @param A the pointer to buffer A
// @param len_A the size of buffer A
// @param B the pointer to buffer B
// @param len_B the size of buffer B
void preset_alloc_set_buffers(void* A, size_t len_A, void* B, size_t len_B);

// Littarly uses the given buffesr and prevides them if the alignment and size match
// @param alignment The alignment required (exist to stop werid bugs)
// @param size The size of the allocation required
void* preset_alloc_aligned_alloc(size_t alignment, size_t size);

// Frees the buffers
// @param p Pointer to free
void preset_alloc_free(void* p);




#endif