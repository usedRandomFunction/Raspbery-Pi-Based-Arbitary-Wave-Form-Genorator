#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>


// Calcuates size of a null terminated string
// @param str The string to calcalate the size of
// @return The size of the string
size_t strlen(const char* str);

// Copys the the src string untill '\0' is reached
// @param src The string to copy
// @param dest_size the size of the destination buffer
// @param dst The destination to copy to
// @return Zero on success or non-zero on failer
int strcpy_s(const char* src, size_t dest_size, char* dst);

// Appends src to dst
// @param src The string to copy
// @param dest_size the size of the destination buffer
// @param dst the string that gets appened to
// @return Zero on success or non-zero on failer
int strcat_s(char* dst, size_t dest_size, const char* src);

// Compares Two null terminated strings
// @param str first string to compare
// @param str second string to compare
// @return < 0 If the first byte that doesn't mach follows *str < *str (8 bit) amd > 0 for the other case. 0 for equals
int strcmp(const char* str1, const char* str2);

// Find the first occurrence of a character in a string
// @param str A pointer to a NULL terminated string
// @param ch The character to shearch for
// @return Pointer to first occurrenc or NULL
char* strchr(const char* str, int ch);

// Find the last occurrence of a character in a string
// @param str A pointer to a NULL terminated string
// @param ch The character to shearch for
// @return Pointer to last occurrenc or NULL
char* strrchr(const char* str, int ch);

#endif