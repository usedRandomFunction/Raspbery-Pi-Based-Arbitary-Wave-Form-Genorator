#if USE_MINI_UART == 0 || !defined(USE_MINI_UART)

#include "io/memoryMappedIO.h"
#include "lib/preset_alloc.h"
#include "io/propertyTags.h"
#include "lib/clocks.h"
#include "lib/timing.h"
#include "io/gpio.h"
#include "io/uart.h"

#include <stdbool.h>

bool uart_initall_init_has_occured = false;

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
	wait_cycles(150);
 
	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	wait_cycles(150);
 
	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);
 
	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);

	uart_set_base_freqency(UART_BASE_CLOCK_FREQUENCY, baudrate);
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_set_base_freqency(int freqenecy, int baudrate)
{
	if (uart_initall_init_has_occured == false)
	{
		unsigned int  __attribute__((aligned(16))) mbox_buffer[9];
		preset_alloc_set_buffers(&mbox_buffer, sizeof(mbox_buffer), &mbox_buffer, 28);

		// Set UART base clock
		set_clock_rate_given_alloc_functions(PROPERTY_TAG_CLOCK_ID_UART, freqenecy, preset_alloc_aligned_alloc, preset_alloc_free);

		uart_initall_init_has_occured = true;
	}
	else
	{
		set_clock_rate(PROPERTY_TAG_CLOCK_ID_UART, freqenecy);
	}

	float divider = freqenecy / (16.0f * baudrate);
	int ibrd = (int)divider;
	mmio_write(UART0_IBRD, ibrd);
	mmio_write(UART0_FBRD, (int)((divider - ibrd) * 64.0f + 0.5f));
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

int uart_poll()
{
	if (mmio_read(UART0_FR) & (1 << 4))
		return 0xFFFF;

	return mmio_read(UART0_DR);
}

#endif