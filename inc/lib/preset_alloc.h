#ifndef PRESET_ALLOC_H
#define PRESET_ALLOC_H

#include <stddef.h>

// Since the property tag functions use alloc these functions exist just to
// Let the property tags be used

#ifdef __cplusplus
extern "C" {
#endif

// Used to set the two buffers
// The idea is that alloca is used, in the parrent function and this sets up the buffers
// the inner functions use this
void preset_alloc_set_buffers(void* A, size_t len_A, void* B, size_t len_B);

// Littarly uses the given buffesr and prevides them if the alignment and size match
void* preset_alloc_aligned_alloc(size_t alignment, size_t size);

void preset_alloc_free(void* p);

#ifdef __cplusplus
}
#endif


#endif