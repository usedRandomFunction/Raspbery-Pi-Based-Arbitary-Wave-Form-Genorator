#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define STR_IPLM(s) #s
#define ToString(s) STR_IPLM(s)
extern "C" {
#endif

// Calcuates size of a null terminated string
// @param str The string to calcalate the size of
// @return The size of the string
size_t strlen(const char* str);

// Copys the the src string untill '\0' is reached
// @param src The string to copy
// @param dst The destination to copy to
void strcpy(const char* src, char* dst);

// Appends src to dst
// @param src The string to copy
// @param dst the string that gets appened to
void strcat(char* dst, const char* src);

// @param digit the digit to convert
// @return A single hexdigiet [0, F]
char hex_digiet(uint8_t digit);

// Writes a hex string with out leading "0x"
// @param max_digits The maxium number of digits that will be writen, up to sizeof(size_t) * 2
// @param str A pointer to a empty string to write to (will be null termenated)
// @return the number of digits written
uint8_t hex_size_t(size_t number, char* str, uint8_t max_digits);

// Writes a hex string with out leading "0x" or leading zeros
// @param max_digits The maxium number of digits that will be writen, up to sizeof(size_t) * 2
// @param str A pointer to a empty string to write to (will be null termenated)
// @return the number of digits written
uint8_t hex_size_t_with_out_leading_zeros(size_t number, char* str, uint8_t max_digits);

#ifdef __cplusplus
}
#endif

#endif