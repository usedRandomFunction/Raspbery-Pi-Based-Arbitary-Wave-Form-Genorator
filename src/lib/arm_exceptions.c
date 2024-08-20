#include "lib/arm_exceptions.h"

#include "io/printf.h"

__attribute__((noinline)) void kernel_panic()
{
	printf("\nkernel panic!\nBegin function trace:\n");

    printf("0 : %x\n", __builtin_return_address(0));
    printf("1 : %x\n", __builtin_return_address(1));
    printf("2 : %x\n", __builtin_return_address(2));
    printf("3 : %x\n", __builtin_return_address(3));
    printf("4 : %x\n", __builtin_return_address(4));

	asm volatile ("wfe");
	while (1)
	{
		// Halt
	}
}

// Heavly based of https://github.com/bztsrc/raspi3-tutorial/blob/master/11_exceptions/exc.c

void arm_exception_handler(unsigned long type) // TODO maby print x0-x30 for more usesfuly data
{
    printf("arm exception detected!\n");
    // print out interruption type
    switch(type) {
        case 0: printf("\nSynchronous"); break;
        case 1: printf("\nIRQ"); break;
        case 2: printf("\nFIQ"); break;
        case 3: printf("\nSError"); break;
    }
    printf(": ");

    size_t reg;
    asm volatile ( "mrs %0, esr_el1" : "=r"(reg));

    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch(reg>>26) {
        case 0b000000: printf("Unknown"); break;
        case 0b000001: printf("Trapped WFI/WFE"); break;
        case 0b001110: printf("Illegal execution"); break;
        case 0b010101: printf("System call"); break;
        case 0b100000: printf("Instruction abort, lower EL"); break;
        case 0b100001: printf("Instruction abort, same EL"); break;
        case 0b100010: printf("Instruction alignment fault"); break;
        case 0b100100: printf("Data abort, lower EL"); break;
        case 0b100101: printf("Data abort, same EL"); break;
        case 0b100110: printf("Stack alignment fault"); break;
        case 0b101100: printf("Floating point"); break;
        default: printf("Unknown"); break;
    }
    // decode data abort cause
    if(reg>>26==0b100100 || reg>>26==0b100101) {
        printf(", ");
        switch((reg>>2)&0x3) {
            case 0: printf("Address size fault"); break;
            case 1: printf("Translation fault"); break;
            case 2: printf("Access flag fault"); break;
            case 3: printf("Permission fault"); break;
        }
        switch(reg&0x3) {
            case 0: printf(" at level 0"); break;
            case 1: printf(" at level 1"); break;
            case 2: printf(" at level 2"); break;
            case 3: printf(" at level 3"); break;
        }
    }
    // dump registers
    asm volatile ( "mrs %0, esr_el1" : "=r"(reg));
    printf(":\n  ESR_EL1 %x", reg);

    asm volatile ( "mrs %0, elr_el1" : "=r"(reg));
    printf(" ELR_EL1 (falt address) %x", reg);

    asm volatile ( "mrs %0, spsr_el1" : "=r"(reg));
    printf("\n SPSR_EL1 %x", reg);

    asm volatile ( "mrs %0, far_el1" : "=r"(reg));
    printf(" FAR_EL1 %x\n", reg);

    kernel_panic();
}