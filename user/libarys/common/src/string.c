#include "common/string.h"

size_t strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s)
            ;
    return (s - str);
}

int strcpy_s(const char* src, size_t dest_size, char* dst)
{
    char* end = dst + dest_size;
    for ( ; *src != '\0'; src++, dst++)
    {
        if (dst == end)
            return -1;

        *dst = *src;
    }
    *dst = '\0';

    return 0;
}

int strcat_s(char* dst, size_t dest_size, const char* src)
{
    int len = (int)strlen(dst);
    return strcpy_s(src, dest_size, dst + len);
}

int strcmp(const char* str1, const char* str2)
{
    const uint8_t* buf_A = (uint8_t*)str1;
    const uint8_t* buf_B = (uint8_t*)str2;

    while (*buf_A != '\0' || *buf_B != '\0') // We dont stop on the first zero as this way it will return correctly
    {
        int diff = *buf_B++ - *buf_A++;
        
        if (diff != 0)
            return diff;
    }

    return 0;
}

char* strchr(const char* str, int ch)
{
    while (*str != '\0')
        if (*str++ == (char)ch)
            return *(char**)(&str) - 1;

    return NULL;
}

char* strrchr(const char* str, int ch)
{
    size_t size = strlen(str);

    const char* ptr = str + size;

    while (ptr-- != str)
        if (*ptr == (char)ch)
            return *(char**)(&ptr);

    return NULL;
}