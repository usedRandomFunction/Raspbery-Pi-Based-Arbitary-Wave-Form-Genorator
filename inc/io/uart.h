#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inintallizeds a UART on pin 14, 15 with baudrate as a varible
void uart_init(int baudrate);

// Writes a single charecter to the UART
void uart_putc(unsigned char c);

// Reads a signaler charecter from the UART
char uart_getc();

// Writes a signed integer to the UART
void uart_puti(int integer);

// Writes a null termaited string to the UART
void uart_puts(const char* str);

// Writes an unsigned integer to the UART
void uart_putui(unsigned int integer);

// Writes float into the UART
void uart_putf(float number, uint8_t decimals);

// Writes a pointer to the UART as a hexidecimal number
inline void uart_put_ptr(const void* const ptr);

// Writes a single hexdighit [0, F] to the UART
void uart_put_hexdigit(uint8_t digit);

// Writes a null termaited string to the UART, in reverse order
// i.e start printing a len - 1 and end at zero
void uart_puts_reversed(const char* str);

// Writes a unsigned number to the UART in hex, with leading zeros and a '0x' prefix
void uart_put_number_as_hex(size_t number);

// Writes a buffer of size length to the UART
void uart_put_buffer(const void* buffer, size_t length);

// Writes the memory to the uart formated like the output simuler to hex dump
void uart_put_memory_dump_formated(void* ptr, size_t size);

// Writes a pointers address to the UART in hex, without leading zeros and a '0x' prefix
inline void uart_put_ptr_without_leading_zeros(const void* const ptr);

// Writes a unsigned number to the UART in hex, without leading zeros and a '0x' prefix
void uart_put_number_as_hex_without_leading_zeros(size_t number);


inline void uart_put_ptr(const void* const ptr)
{
    uart_put_number_as_hex(*(size_t*)&ptr);
}

inline void uart_put_ptr_without_leading_zeros(const void* const ptr)
{
    uart_put_number_as_hex_without_leading_zeros(*(size_t*)&ptr);
}

#ifdef __cplusplus
}
#endif

#endif