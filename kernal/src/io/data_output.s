.section .text

.extern _dac_output_start_internal

.global dac_output_start
dac_output_start:
    sub	    sp,         sp,     #16 * 5         // We start by saving x19 to x30
    stp	    x19,        x20,    [sp, #16 * 0]
	stp	    x21,        x22,    [sp, #16 * 1]
	stp	    x23,        x24,    [sp, #16 * 1]
	stp	    x25,        x26,    [sp, #16 * 2]
	stp	    x27,        x28,    [sp, #16 * 3]
	stp	    x29,        x30,    [sp, #16 * 4]

    ldr     x3,     =dac_output_return_stack    // Save the stack pointer now
    mov     x4,     sp
    str     x4,     [x3]
    

    bl      _dac_output_start_internal


.global dac_output_end
dac_output_end:
    ldr     x1,     =dac_output_return_stack
    ldr     x2,     [x1]     // Load the original stack address's address (english go brrr)
    str     xzr,    [x1]     // Zero it
    cmp     x2,     xzr
    beq     .dac_output_end_return      // Just return normaly is its zero
    mov     sp,     x2       // Restore it

    ldp	    x19,        x20,    [sp, #16 * 0]   // Restore the Callee-Saved Registers (x19 to x30)
	ldp	    x21,        x22,    [sp, #16 * 1]
	ldp	    x23,        x24,    [sp, #16 * 1]
	ldp	    x25,        x26,    [sp, #16 * 2]
	ldp	    x27,        x28,    [sp, #16 * 3]
	ldp	    x29,        x30,    [sp, #16 * 4]

    add     sp,         sp,     #16 * 5         // and fix the stack
.dac_output_end_return:
    ret



.section .bss
.align 3 
 .global dac_output_return_stack
dac_output_return_stack:
.skip 8 