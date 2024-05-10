#ifndef _H
#define _H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define alloca(x) __builtin_alloca((x))
#else
#error failed to wind alloca
#endif

void* aligned_alloc(size_t alignment, size_t size);
void* malloc(size_t size);
void free(void* p);


#ifdef __cplusplus
}
#endif

#endif