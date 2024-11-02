#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



extern bool uart_initall_init_has_occured;

// Inintallizes a UART on pin 14, 15
// @param baudrate The baudrate of the uart
void uart_init(int baudrate);

// Sets the base freqency of the UART, and reconfigures the UART baudrate
// @param freqenecy the base freqenecy in hertz
// @param baudrate The baudrate of the uart
void uart_set_base_freqency(int freqenecy, int baudrate);

// Writes a single charecter to the UART
// @param c The charecter to put on the UART
void uart_putc(unsigned char c);

// Reads a signaler charecter from the UART
// @return The charecter read from the UART
char uart_getc();

// Reads a signaler charecter from the UART but doesn't wait for one to be avaible
// @return The charecter read from the UART or 0xFFFF if none are avaible
int uart_poll();

// // Writes a signed integer to the UART
// // @param integer The number to put in the UART
// void uart_puti(int integer);

// Writes a null termaited string to the UART
// @param str The string to put on the UART
void uart_puts(const char* str);

// Enables UART_RTINTR (FIFO 1/4)
void enable_uart_receive_interupt();

// Disables UART_RTINTR
void disable_uart_receive_interupt();

// Enables IRQ 57
void enable_uart_interupts();

// Disables IRQ 57
void disable_uart_interupts();

// // Writes an unsigned integer to the UART
// // @param integer The number to put in the UART
// void uart_putui(unsigned int integer);

// // Writes float into the UART
// // @param number The number to put in the UART
// void uart_putf(float number, uint8_t decimals);

// // Writes a pointer to the UART as a hexidecimal number
// // @param ptr the pointer to convert to a number and put on the UART
// inline void uart_put_ptr(const void* const ptr);

// // Writes a single hexdighit [0, F] to the UART
// // @param digit the digit to convert
// void uart_put_hexdigit(uint8_t digit);

// // Writes a null termaited string to the UART, in reverse order
// // i.e start printing a len - 1 and end at zero
// // @param str The string to put on the UART
// void uart_puts_reversed(const char* str);

// // Writes a unsigned number to the UART in hex, with leading zeros and a '0x' prefix
// // @param number The number to put in the UART
// void uart_put_number_as_hex(size_t number);

// // Writes a buffer of size length to the UART
// // @param buffer The buffer to be put on the UART
// // @param length The length (in bytes) of the buffer 
// void uart_put_buffer(const void* buffer, size_t length);

// Writes the memory to the uart formated like the output simuler to hex dump
// @param The pointer to the memory to dump
// @param size The ammount of bytes to dump
// void uart_put_memory_dump_formated(void* ptr, size_t size);

// // Writes a pointers address to the UART in hex, without leading zeros and a '0x' prefix
// // @param ptr the pointer to convert to a number and put on the UART
// inline void uart_put_ptr_without_leading_zeros(const void* const ptr);

// // Writes a unsigned number to the UART in hex, without leading zeros and a '0x' prefix
// // @param number The number to put in the UART
// void uart_put_number_as_hex_without_leading_zeros(size_t number);


// inline void uart_put_ptr(const void* const ptr)
// {
//     uart_put_number_as_hex(*(size_t*)&ptr);
// }

// inline void uart_put_ptr_without_leading_zeros(const void* const ptr)
// {
//     uart_put_number_as_hex_without_leading_zeros(*(size_t*)&ptr);
// }



#endif