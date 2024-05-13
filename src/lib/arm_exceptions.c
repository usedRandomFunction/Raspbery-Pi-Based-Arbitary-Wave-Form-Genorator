#include "lib/arm_exceptions.h"

#include "io/uart.h"

void kernel_panic()
{
	uart_puts("\nkernel panic!\n");

	asm volatile ("wfe");
	while (1)
	{
		// Halt
	}
}

// Heavly based of https://github.com/bztsrc/raspi3-tutorial/blob/master/11_exceptions/exc.c

void arm_exception_handler(unsigned long type) // TODO maby print x0-x30 for more usesfuly data
{
    uart_puts("arm exception detected!\n");
    // print out interruption type
    switch(type) {
        case 0: uart_puts("\nSynchronous"); break;
        case 1: uart_puts("\nIRQ"); break;
        case 2: uart_puts("\nFIQ"); break;
        case 3: uart_puts("\nSError"); break;
    }
    uart_puts(": ");

    size_t reg;
    asm volatile ( "mrs %0, esr_el1" : "=r"(reg));

    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(reg>>26) {
        case 0b000000: uart_puts("Unknown"); break;
        case 0b000001: uart_puts("Trapped WFI/WFE"); break;
        case 0b001110: uart_puts("Illegal execution"); break;
        case 0b010101: uart_puts("System call"); break;
        case 0b100000: uart_puts("Instruction abort, lower EL"); break;
        case 0b100001: uart_puts("Instruction abort, same EL"); break;
        case 0b100010: uart_puts("Instruction alignment fault"); break;
        case 0b100100: uart_puts("Data abort, lower EL"); break;
        case 0b100101: uart_puts("Data abort, same EL"); break;
        case 0b100110: uart_puts("Stack alignment fault"); break;
        case 0b101100: uart_puts("Floating point"); break;
        default: uart_puts("Unknown"); break;
    }
    // decode data abort cause
    if(reg>>26==0b100100 || reg>>26==0b100101) {
        uart_puts(", ");
        switch((reg>>2)&0x3) {
            case 0: uart_puts("Address size fault"); break;
            case 1: uart_puts("Translation fault"); break;
            case 2: uart_puts("Access flag fault"); break;
            case 3: uart_puts("Permission fault"); break;
        }
        switch(reg&0x3) {
            case 0: uart_puts(" at level 0"); break;
            case 1: uart_puts(" at level 1"); break;
            case 2: uart_puts(" at level 2"); break;
            case 3: uart_puts(" at level 3"); break;
        }
    }
    // dump registers
    uart_puts(":\n  ESR_EL1 ");
    uart_put_number_as_hex(reg);

    asm volatile ( "mrs %0, elr_el1" : "=r"(reg));
    uart_puts(" ELR_EL1 ");
    uart_put_number_as_hex(reg);

    asm volatile ( "mrs %0, spsr_el1" : "=r"(reg));
    uart_puts("\n SPSR_EL1 ");
    uart_put_number_as_hex(reg);

    asm volatile ( "mrs %0, spsr_el1" : "=r"(reg));
    uart_puts(" FAR_EL1 ");
    uart_put_number_as_hex(reg);
    uart_puts("\n");

    kernel_panic();
}