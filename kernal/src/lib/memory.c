#include "lib/memory.h"

int memcmp(const void* ptr1, const void* ptr2, size_t n)
{
    const uint8_t* buf_A = ptr1;
    const uint8_t* buf_B = ptr2;

    while (n--)
    {
        int diff = *buf_B++ - *buf_A++;
        
        if (diff != 0)
            return diff;
    }

    return 0;
}

void* memcpy(void* dst, const void* src, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void* memset(void* dst, size_t size, uint8_t value)
{
    uint8_t* true_dst = dst;

    for (int i = 0; i < size; i++)
        true_dst[i] = value;

    return dst;
}

void* memclr(void* dst, size_t size)
{
    uint8_t* true_dst = dst;

    for (size_t i = 0; i < size; i++)
        true_dst[i] = 0;

    return dst;
}

bool is_kernal_memory(const void* ptr)
{
    return ((size_t)ptr) >= KERNEL_MEMORY_PREFIX;
}