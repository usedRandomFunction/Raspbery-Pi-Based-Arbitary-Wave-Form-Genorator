#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

// Compares ptr 1 to pt2
// @param ptr1 first Bufer to compare
// @param ptr2 second Buffer to compare
// @param n Number of bytes to  comapre
// @return < 0 If the first byte that doesn't mach follows *ptr2 < *ptr1 (8 bit) amd > 0 for the opisit, 0 if equal
int memcmp(const void* ptr1, const void* ptr2, size_t n);

// Copys n bytes from src to dst
// @param dst The destination buffer
// @param src The soruce buffer
// @param n The number of bytes to copy
void* memcpy(void* dst, const void* src, size_t n);

// Sets n bytes of dst to value
// @param dst The destination buffer
// @param size The number of bytes to set
// @param value The value to write
// @return a pointer to the destination buffer
void* memset(void* dst, size_t size, uint8_t value);


// Sets n bytes of dst to 0
// @param dst The destination buffer
// @param size The number of bytes to set
// @return a pointer to the destination buffer
void* memclr(void* dst, size_t size);

#endif