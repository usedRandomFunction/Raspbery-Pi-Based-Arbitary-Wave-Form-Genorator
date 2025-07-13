#ifndef BASIC_IO_H
#define BASIC_IO_H

#include <stddef.h>

// Prints the given string to the screen and uart
// @param str String to print
// @returns number of bytes written
unsigned int printf(const char* str, ...);

// Wires to the given string in the same way as printf does
// @param dst Destantion string
// @param size_of_buffer size of dst in bytes
// @param str String to print
// @returns number of bytes written
unsigned int sprintf_s(char *dst, size_t size_of_Buffer, const char* str, ...);

// Puts the given charecter on the screen and uart
// @param c charecter to print
// @return returns EOF if failed
int putchar(char c);

#endif
