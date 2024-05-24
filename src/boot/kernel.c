#include "boot/boardDectetion.h"
#include "io/memoryMappedIO.h"
#include "io/uart.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main(); // The main system Function

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
 
#ifdef AARCH64
// arguments for AArch64
void kernel_main(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
	// Declare as unused
	// (void) dtb_ptr32;
	// (void) x1;
	// (void) x2;
	// (void) x3;
#else
// arguments for AArch32
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;
#endif
{
	int boardType = get_board_type();
	set_mmio_base(boardType);

	#ifdef UART_BAUD_RATE
	uart_init(UART_BAUD_RATE);
	#else
	uart_init(115200);
	#endif

	uart_puts("Started system, board type: ");
	uart_puts(get_board_name(boardType));

	uart_puts("\nException level: ");
    unsigned int reg = 0;
    asm volatile ("mrs %x0, CurrentEL" : "=r" (reg));
    uart_putui(reg >> 2);

	uart_puts("\nStarting main function!\n");
	int result = main();

	uart_puts("Program ended with code: ");
	uart_puti(result);
	uart_puts("!\n");
	while (1)
		uart_putc(uart_getc());
}
