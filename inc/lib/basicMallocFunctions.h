#ifndef BASIC_MALLOC_FUNCTIONS_H
#define BASIC_MALLOC_FUNCTIONS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void free(void* p);
void* malloc(size_t size);
void* aligned_alloc(size_t alignment, size_t size);

#ifdef __cplusplus
}
#endif

#endif