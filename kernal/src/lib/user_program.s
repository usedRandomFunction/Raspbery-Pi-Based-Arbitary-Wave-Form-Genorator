.section .text

.extern system_call_exit_return_stack
.extern system_call_exit_return_address
                               
.equ RETURN_VALUE_MAGIC_NUMBER, 0xF55F433AE4D230F4

zero_all_registers_except_for_x30:
    mov x0,         xzr
    mov x1,         xzr
    mov x2,         xzr
    mov x3,         xzr
    mov x4,         xzr
    mov x5,         xzr
    mov x6,         xzr
    mov x7,         xzr
    mov x8,         xzr
    mov x9,         xzr
    mov x10,        xzr
    mov x11,        xzr
    mov x12,        xzr
    mov x13,        xzr
    mov x14,        xzr
    mov x15,        xzr
    mov x16,        xzr
    mov x17,        xzr
    mov x18,        xzr
    mov x19,        xzr
    mov x20,        xzr
    mov x21,        xzr
    mov x22,        xzr
    mov x23,        xzr
    mov x24,        xzr
    mov x25,        xzr
    mov x26,        xzr
    mov x27,        xzr
    mov x28,        xzr
    mov x29,        xzr

    ret

.global _execute_user_program
_execute_user_program:
    msr sp_el0, x1

    mov x1,         #0x00
    msr spsr_el1,   x1
    
    msr elr_el1,    x0

    sub	sp,         sp,     #16 * 16
	stp	x0,         x1,     [sp, #16 * 0]
	stp	x2,         x3,     [sp, #16 * 1]
	stp	x4,         x5,     [sp, #16 * 2]
	stp	x6,         x7,     [sp, #16 * 3]
	stp	x8,         x9,     [sp, #16 * 4]
	stp	x10,        x11,    [sp, #16 * 5]
	stp	x12,        x13,    [sp, #16 * 6]
	stp	x14,        x15,    [sp, #16 * 7]
	stp	x16,        x17,    [sp, #16 * 8]
	stp	x18,        x19,    [sp, #16 * 9]
	stp	x20,        x21,    [sp, #16 * 10]
	stp	x22,        x23,    [sp, #16 * 11]
	stp	x24,        x25,    [sp, #16 * 12]
	stp	x26,        x27,    [sp, #16 * 13]
	stp	x28,        x29,    [sp, #16 * 14]
    str x30,                [sp, #16 * 15]

    ldr x0,         =system_call_exit_return_stack
    mov x1,         sp
    str x1,         [x0]
    ldr x0,         =system_call_exit_return_address
    adr x1,         ._execute_user_program_return_address
    str x1,         [x0]


    bl zero_all_registers_except_for_x30
    mov x30,        xzr


    eret

._execute_user_program_return_address:
    ldp	x0,         x1,     [sp, #16 * 0]
	ldp	x2,         x3,     [sp, #16 * 1]
	ldp	x4,         x5,     [sp, #16 * 2]
	ldp	x6,         x7,     [sp, #16 * 3]
	ldp	x8,         x9,     [sp, #16 * 4]
	ldp	x10,        x11,    [sp, #16 * 5]
	ldp	x12,        x13,    [sp, #16 * 6]
	ldp	x14,        x15,    [sp, #16 * 7]
	ldp	x16,        x17,    [sp, #16 * 8]
	ldp	x18,        x19,    [sp, #16 * 9]
	ldp	x20,        x21,    [sp, #16 * 10]
	ldp	x22,        x23,    [sp, #16 * 11]
	ldp	x24,        x25,    [sp, #16 * 12]
	ldp	x26,        x27,    [sp, #16 * 13]
	ldp	x28,        x29,    [sp, #16 * 14]
    ldr x30,                [sp, #16 * 15]
    add	sp, sp, #16 * 16

    ret


.global _execute_function_as_user_program
_execute_function_as_user_program:      
    sub	    sp,         sp,     #16 * 6         // We start by saving x19 to x30, spsr_el1, and elr_el1
    stp	    x19,        x20,    [sp, #16 * 0]
	stp	    x21,        x22,    [sp, #16 * 1]
	stp	    x23,        x24,    [sp, #16 * 1]
	stp	    x25,        x26,    [sp, #16 * 2]
	stp	    x27,        x28,    [sp, #16 * 3]
	stp	    x29,        x30,    [sp, #16 * 4]
    mrs     x1,         spsr_el1
    mrs     x2,         elr_el1
    stp     x1,         x2,     [sp, #16 * 5]

    mov     x1,         1 << 7                  // IRQ mask bit will be set so we dont get recursive interupts here
    msr     spsr_el1,   x1                      // Program state for user programs
    msr     elr_el1,    x0                      // Set return address

    ldr     x0,         =_execute_function_as_user_program_return_stack
    mov     x1,         sp                      // Save the stack so we dont lose it
    str     x1,         [x0]

    bl      zero_all_registers_except_for_x30

    ldr     x30,    =RETURN_VALUE_MAGIC_NUMBER
    eret

.global _return_from_user_function
_return_from_user_function:
    ldr     x0,         =_execute_function_as_user_program_return_stack
    ldr     x1,         [x0]
    mov     sp,         x1                      // Restore the stack

    ldp	    x19,        x20,    [sp, #16 * 0]   // Restore the Callee-Saved Registers
	ldp	    x21,        x22,    [sp, #16 * 1]
	ldp	    x23,        x24,    [sp, #16 * 1]
	ldp	    x25,        x26,    [sp, #16 * 2]
	ldp	    x27,        x28,    [sp, #16 * 3]
	ldp	    x29,        x30,    [sp, #16 * 4]

    ldp     x1,         x2,     [sp, #16 * 5]   // And spsr_el1, elr_el1
    msr     spsr_el1,           x1              // Program state for user programs
    msr     elr_el1,            x2              // Set return address

    add     sp,         sp,     #16 * 6

    ret

.section .bss
.align 3 
 .global _execute_function_as_user_program_return_stack
_execute_function_as_user_program_return_stack:
.skip 8 

