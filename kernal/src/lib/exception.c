#include "lib/exceptions.h"

#include "lib/user_program.h"
#include "io/putchar.h"
#include "io/keypad.h"
#include "io/printf.h"

extern void* user_program_x30;

// printf infomation about a failed arm exception
// @param type The type of exception
static void s_print_arm_exception_details(unsigned long type);

// !================== NOTE ==================!
// These functions are not static, despite being 
// not being shown in the header, as they are only
// use by vectors.s whitch cant used header files
// !================== NOTE ==================!

// Used by vectors.s to handle arm exceptions from user programs
// @param type The type of exception
// @note this function will terminate the current user program
// @warning Do not call this function it is to be called only vectors.s
void user_arm_exception_handler(unsigned long type);

// Used by vectors.s to handle arm exceptions from user programs
// @param type The type of exception
// @note this function will halt execution.
// @warning Do not call this function it is to be called only vectors.s
void kernal_arm_exception_handler(unsigned long type);

__attribute__((noinline)) void kernel_panic()
{
	printf("\nkernel panic!\n");

	asm volatile ("wfe");
	while (1)
	{
		// Halt
	}
}

void generic_user_exception(const char* fmt, ...)
{
    printf("\n\n\n\n!======== Excpetion thorwn while hanlding system call for user ========!\n\n");

    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);

    size_t elr_el1;
    asm volatile ( "mrs %0, elr_el1" : "=r"(elr_el1));
    
    printf("elr_el1: 0x%x (sys call return addresss)\n", elr_el1);
    printf("x30: 0x%x  (user function return address)\n", user_program_x30);

    printf("Terminating current user program!\n\n");

    putchar('!');
    for (int i = 0; i < 70; i++)
        putchar('=');

    printf("!\n");
    on_user_program_exiting();          // Resets keypad and display. Needed incase appillcation has turned these off.
    asm volatile("msr DAIFClr, #2");    // Enable IRQs
    halt_and_wait_from_user_input();
    printf("\n\n");

    terminate_current_user_program();
}

void user_arm_exception_handler(unsigned long type)
{
    if (check_if_instruction_abort_is_user_program_function_return())
        return;

    printf("\n\n\n\n!======== Arm excpetion detected in %s ========!\n\n", "user mode");

    s_print_arm_exception_details(type);

    printf("x30: 0x%x  (user function return address)\n", user_program_x30);

    printf("Terminating current user program!\n\n");

    putchar('!');
    for (int i = 0; i < 53; i++)
        putchar('=');
    printf("!\n");
    on_user_program_exiting();          // Resets keypad and display. Needed incase appillcation has turned these off.
    asm volatile("msr DAIFClr, #2");    // Enable IRQs
    halt_and_wait_from_user_input();
    printf("\n\n");
    terminate_current_user_program();
}

void kernal_arm_exception_handler(unsigned long type)
{
    printf("\n\n\n\n!======== Arm excpetion detected in %s ========!\n\n", "kernal mode");
    s_print_arm_exception_details(type);

    printf("Current return address: 0x%x\n", __builtin_return_address(0));

    if (is_user_program_active())
    {
        printf("Terminating current user program!\n\n\n\n\n");
        printf("Warning: due to the kernal function ending where\n it is not supost to, a memory leak may have occured!\n");
    
        putchar('!');
        for (int i = 0; i < 55; i++)
            putchar('=');
        printf("!\n\n\n");

        terminate_current_user_program();
    }



   kernel_panic();
}

// Heavly based of https://github.com/bztsrc/raspi3-tutorial/blob/master/11_exceptions/exc.c
static void s_print_arm_exception_details(unsigned long type) // TODO maby print x0-x30 for more usesfuly data
{
    // print out interruption type
    switch(type) {
        case 0: printf("Synchronous"); break;
        case 1: printf("IRQ"); break;
        case 2: printf("FIQ"); break;
        case 3: printf("SError"); break;
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
    printf(":\n  ESR_EL1 0x%x", reg);

    asm volatile ( "mrs %0, elr_el1" : "=r"(reg));
    printf(" ELR_EL1 (Instruction address) 0x%x", reg);

    asm volatile ( "mrs %0, spsr_el1" : "=r"(reg));
    printf("\n SPSR_EL1 0x%x", reg);

    asm volatile ( "mrs %0, far_el1" : "=r"(reg));
    printf(" FAR_EL1 (Falt address) 0x%x\n", reg);
}
