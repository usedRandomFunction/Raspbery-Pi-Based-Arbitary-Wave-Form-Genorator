#ifndef UART_H
#define UART_H

// Puts on given charecter to the uart
// @param c charecter to print
// @return returns EOF if failed
int uart_putc(char c);

// Gets a charecter from the UART, if none are avaible will wait for one
// @return The charecter from the UART
char uart_getc();

// Gets a charecter from the UART, if none are avaible will return 0xFFFF
// @return The charecter from the UART or 0xFFFF
int uart_poll();

#endif