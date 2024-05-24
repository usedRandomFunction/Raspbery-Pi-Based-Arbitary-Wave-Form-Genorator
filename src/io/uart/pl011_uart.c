#if USE_MINI_UART == 0 || !defined(USE_MINI_UART)

#include "io/memoryMappedIO.h"
#include "lib/timing.h"
#include "io/mailbox.h"
#include "io/gpio.h"
#include "io/uart.h"

#define UART_BASE_FREQENECY_MHZ 3

// A Mailbox message with set clock rate of PL011 to 3MHz tag
volatile unsigned int  __attribute__((aligned(16))) mbox[9] = {
    9*4, 0, 0x38002, 12, 8, 2, UART_BASE_FREQENECY_MHZ * 1000000, 0 ,0
};

void uart_init(int baudrate)
{
	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);

	// Setup the GPIO pin 14 && 15.

	// Set Function register
	gpio_function_select(14, GPFSEL_Alternate0);
	gpio_function_select(15, GPFSEL_Alternate0);

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);
 
	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
 
	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);
 
	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);

	// Set UART base clock
	mailbox_write_read_alligedAddress((void*)&mbox, 8);


	float divider = UART_BASE_FREQENECY_MHZ * 1000000.0f / (16.0f * baudrate);
	int ibrd = (int)divider;
	mmio_write(UART0_IBRD, ibrd);
	mmio_write(UART0_FBRD, (int)((divider - ibrd) * 64.0f + 0.5f));
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}
 
char uart_getc()
{
    // Wait for UART to have received something.
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

#endif