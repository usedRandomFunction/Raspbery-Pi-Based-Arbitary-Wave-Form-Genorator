.section .text

.extern SYSTEM_CALL_EXIT_RETURN_STACK
.extern SYSTEM_CALL_EXIT_RETURN_ADDRESS

.global user_program_internal_use_program_excute
user_program_internal_use_program_excute:
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

    ldr x0,         =SYSTEM_CALL_EXIT_RETURN_STACK
    mov x1,         sp
    str x1,         [x0]
    ldr x0,         =SYSTEM_CALL_EXIT_RETURN_ADDRESS
    adr x1,         .user_program_internal_use_program_excute_return_address
    str x1,         [x0]


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
    mov x30,        xzr


    eret

.user_program_internal_use_program_excute_return_address:
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
