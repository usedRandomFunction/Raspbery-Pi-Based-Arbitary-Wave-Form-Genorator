#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
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
// @return < 0 If the first byte that doesn't mach follows *str < *str (8 bit) amd > 0 for the o
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

// Simple sting to number function
// Takes the given string and turns it into a uint64_t
// suports binary, base 10 and hexadecimal
// @param str String to convert
// @param end (Optional) used to show the end of the data
// @param error (Optional) used to show if a error occured (Ture if error)
// @return The value
uint64_t string_to_u64(char* str, char* end, bool* error);

// Simple sting to number function
// Takes the given string and turns it into a int64_t
// suports binary, base 10 and hexadecimal
// @param str String to convert
// @param end (Optional) used to show the end of the data
// @param error (Optional) used to show if a error occured (Ture if error)
// @return The value
int64_t string_to_s64(char* str, char* end, bool* error);

// Take from http://www.cse.yorku.ca/~oz/hash.html
// Simple hash function
// @param str String to hash
// @return hash
size_t djb2_hash(const char *str);

// Take from http://www.cse.yorku.ca/~oz/hash.html
// Simple hash function but convvects the string to upper case first
// @param str String to hash
// @return hash
size_t djb2_hash_uppercase(const char *str);

// Take from http://www.cse.yorku.ca/~oz/hash.html
// Simple hash function but for non-null terminated strings
// @param str String to hash
// @return hash
size_t djb2_hash_of_size(const char *str, size_t n);

// Converts the given character to upper case
// @param ch Character to convert
// @return ch but upper case
char toupper(char ch);

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



#endif