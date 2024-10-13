#include "lib/string.h"
#include "lib/math.h"

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

char hex_digiet(uint8_t digit)
{
    if (digit > 9)
		digit += 0x7;
	digit += 0x30;

	return digit;
}

uint64_t string_to_u64(char* str, char* end, bool* error)
{
    uint64_t value = 0;
    char base = 10;

    if (error)
        *error = false;

    if (end - str > 2 && *str == '0')
    {
        if (*(str + 1) == 'b' || *(str + 1) == 'B')
        {
            base = 2;
            str += 2;
        }
        else if (*(str + 1) == 'x' || *(str + 1) == 'X')
        {
            base = 16;
            str += 2;
        }
    }

    while (*str != '\0' && str != end)
    {
        char c = *str++;
                                                // Turn c into a value from 0 to base
        if (c < 0x30)                           // Less then 0 error
        {
            if (error)
                *error = true;

            return 0;
        }

        if (c <= 0x39)                          // 0 to 9
            c -= 0x30;
        else
        {
            if (c >= 0x41 && c <= 0x5A)         // Upper case
                c -= 0x37;
            else if (c >= 0x61 && c <= 0x7A)    // Lower case
                c -= 0x57;
            else                                // Not a valid digit
            {
                if (error)
                    *error = true;

                return 0;
            }
        }

        if (c > base)                           // Digit larger then base?
        {
            if (error)
                *error = true;

            return 0;
        }

        value *= base;
        value += c;
    }
    
    return value;
}

int64_t string_to_s64(char* str, char* end, bool* error)
{
    if (*str == '-')
        return  -((int64_t)string_to_u64(str + 1, end, error));

    return  (int64_t)string_to_u64(str, end, error);
}

size_t djb2_hash(const char *str)
{
    size_t hash = 5381;
    int c;

    while ((c = (uint8_t)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

size_t djb2_hash_uppercase(const char *str)
{
    size_t hash = 5381;
    int c;

    while ((c = (uint8_t)toupper(*str++)))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

size_t djb2_hash_of_size(const char *str, size_t n)
{
    size_t hash = 5381;
    int c;

    while (n--)
    {
        c = (uint8_t)*str++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

char toupper(char ch)
{
    if (ch >= 0x61 && ch <= 0x7A)
        return ch - 0x20;
    
    return ch;
}
// #include "io/uart.h"

uint8_t hex_size_t(size_t number, char* str, uint8_t max_digits)
{
    int digits = max(min(max_digits, sizeof(number) * 2), 1); // Dont let the program have digits = 0
	for (int i = digits * 4 - 4; i >= 0; i -= 4, str++)
    {
		*str = hex_digiet((number >> i) & 0xF);
    }

    *str = '\0';
    return digits;
}


uint8_t hex_size_t_with_out_leading_zeros(size_t number, char* str, uint8_t max_digits)
{
	int startPossition = 0;

	for (int i = 0; i < sizeof(number) * 8; i += 4)
	{
		if (number & (0xF << i))
		{
			startPossition = i;
		}
	}
    startPossition = min(startPossition, max_digits * 4);

	for (int i = startPossition; i >= 0; i -= 4, str++)
		*str = hex_digiet((number >> i) & 0xF);

    *str = '\0';
    return startPossition / 4;
}
