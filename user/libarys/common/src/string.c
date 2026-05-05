#include "common/string.h"
#include "common/math.h"

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

char* ftoa_s(double in, char* dst, int precision, size_t dest_size)
{
   if (dest_size <= 1)
        return NULL;        // IDk what else to do in this case
    dest_size--;            // This inculdes the null terminatior 
    
    if (in < 0)
    {
        if (dest_size > 1)
        {
            *dst++ = '-';
            dest_size--;
        }

        in = -in;
    }

    int intiger_digits = (int)ceil(log10(in)) + 1;

    if (precision < 0)
        precision = dest_size - 1;  
    
    if (intiger_digits > dest_size)
    {
        *dst++ = 'X';
        *dst = '\0';
        return dst;
    }

    if (precision > (dest_size - intiger_digits))
        precision = dest_size - intiger_digits;

    double magnitude = pow(10, intiger_digits - 1); 
    
    do
    {
        int digit = (int)floor(fmod(in / magnitude, 10));
        magnitude /= 10;
        //in /= 10;

        *dst++ = (char)(0x30 + digit);
    }
    while (intiger_digits--); 

    if (precision > 0)
    {
        in = fmod(in, 1) * 10;
        
        *dst++ = '.';

        while (precision--)
        {
            double ones = floor(in);
            int digit = (int)ones;

            *dst++ = (char)(digit + 0x30);

            in -= ones;
            in *= 10;
        }
    }

    *dst = '\0';
    return dst;
}
