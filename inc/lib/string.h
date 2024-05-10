#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define STR_IPLM(s) #s
#define ToString(s) STR_IPLM(s)
extern "C" {
#endif

// Returns the size of a null terminated string
size_t strlen(const char* str);

// Copys the the src string untill '\0' is reached
void strcpy(const char* src, char* dst);

// Copys src to dst after the first occuaence of '\0;
void strcat(char* dst, const char* src);

// Retuns a single hexdighit [0, F]
char hex_digiet(uint8_t digit);

// Writes a hex string with out leading "0x"
// max_digits is the maxium number of digits that will be writen, up to sizeof(size_t) * 2
// modifys str, and the charecter after the ends is set to '\0'
// returns the number of digits written
uint8_t hex_size_t(size_t number, char* str, uint8_t max_digits);

// Writes a hex string with out leading "0x" or leading zeros
// max_digits is the maxium number of digits that will be writen, up to sizeof(size_t) * 2
// modifys str, and the charecter after the ends is set to '\0'
// returns the number of digits written
uint8_t hex_size_t_with_out_leading_zeros(size_t number, char* str, uint8_t max_digits);

#ifdef __cplusplus
}
#endif

#endif