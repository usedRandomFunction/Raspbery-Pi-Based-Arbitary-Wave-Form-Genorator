#include "io/putchar.h"
#include "io/uart.h"

int putchar(int ch)
{
    uart_putc((uint8_t)ch);
}