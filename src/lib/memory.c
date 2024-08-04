#include "lib/memory.h"

#include "io/uart.h"

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