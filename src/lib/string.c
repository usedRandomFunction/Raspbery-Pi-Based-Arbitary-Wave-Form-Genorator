#include "lib/string.h"
#include "lib/math.h"

size_t strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s)
            ;
    return (s - str);
}

void strcpy(const char* src, char* dst)
{
    for ( ; *src != '\0'; src++, dst++)
        *dst = *src;
    *dst = '\0';
}

void strcat(char* dst, const char* src)
{
    int len = (int)strlen(dst);
    strcpy(src, dst + len);
}

char hex_digiet(uint8_t digit)
{
    if (digit > 9)
		digit += 0x7;
	digit += 0x30;

	return digit;
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
