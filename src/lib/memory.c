#include "lib/memory.h"

#include "io/uart.h"

#pragma GCC push_options // Beocuse gcc is r and will break all of the code if dont
#pragma GCC optimize ("O0")

void* memcpy(void* dst, const void* src, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

#pragma GCC pop_options

void* memset(void* dst, size_t size, uint8_t value)
{
    uint8_t* true_dst = dst;

    for (int i = 0; i < size; i++)
        true_dst[i] = value;

    return dst;
}