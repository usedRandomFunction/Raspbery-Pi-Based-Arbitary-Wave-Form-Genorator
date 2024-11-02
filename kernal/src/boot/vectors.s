.section ".text"

.global _vectors

.extern system_call_undefined_handler
.extern kernal_arm_exception_handler
.extern user_arm_exception_handler
.extern system_call_exit

.macro vector_table_entry function
.align 7
b \function
.endm

.macro exception_handler_kernal_error id
.align  7
mov x0, \id
b       kernal_arm_exception_handler
.endm

.macro exception_handler_user_error id
.align  7
mov x0, \id
b        user_arm_exception_handler
.endm

.macro enter_from_syscall
sub	sp,     sp,     #16
stp x30,    x29,    [sp]
ldr x29,    =user_program_x30
str x30,    [x29]               // Used by some trace stuff to help with debuging
bl enter_from_syscall_internal
.endm

.macro enter_from_interupt
sub	sp,     sp,     #16
stp x30,    x29,    [sp]
bl enter_from_interupt_internal
.endm

.align	11
_vectors:
    // Exceptions for EL1t
    exception_handler_kernal_error #0 // synchronous
    vector_table_entry .irq           // IRQ
    exception_handler_kernal_error #2 // FIQ
    exception_handler_kernal_error #3 // SError

    // Exceptions for EL1h
    exception_handler_kernal_error #0 // synchronous
    vector_table_entry .irq           // IRQ
    exception_handler_kernal_error #2 // FIQ
    exception_handler_kernal_error #3 // SError

    // Exceptions for EL0 (AArch64)
    vector_table_entry .el0_svn // synchronous
    vector_table_entry .irq           // IRQ
    exception_handler_user_error #2 // FIQ
    exception_handler_user_error #3 // SError

    // Exceptions for EL0 (AArch32)
    exception_handler_user_error #0 // synchronous
    vector_table_entry .irq         // IRQ
    exception_handler_user_error #2 // FIQ
    exception_handler_user_error #3 // SError

.irq:
    enter_from_interupt

    bl generic_irq_handler

    b exit_from_interupt


.el0_svn:
    enter_from_syscall
    mrs	x24, esr_el1				// read the syndrome register
	lsr	x24, x24, #26		// exception class
	cmp	x24, #0x15			// SVC in 64-bit state
	b.eq	.el0_svc
    mov x0, 0
    b       user_arm_exception_handler

.el0_svc:
    cmp x8,     19              // Number of syscalls
    bge .el0_svc_failed
    ldr x9,     =system_call_table
    lsl x10,    x8,     #3      // Sames as x8 = x8 * 8 (2^3)
    add x9,     x9,     x10
    ldr x10,    [x9]
    blr x10
    b exit_from_syscall


.el0_svc_failed:
    b       system_call_undefined_handler

enter_from_interupt_internal:
    sub	sp, sp, #16 * 15

    str x0,             [sp, #16 * 0]
    stp x1,     x2,     [sp, #16 * 1]
    stp x3,     x4,     [sp, #16 * 2]
    stp x5,     x6,     [sp, #16 * 3]
    stp x7,     x8,     [sp, #16 * 4]
    stp x9,     x10,    [sp, #16 * 5]
    stp x10,    x11,    [sp, #16 * 6]
    stp x12,    x13,    [sp, #16 * 7]
    stp x14,    x15,    [sp, #16 * 8]
    stp x16,    x17,    [sp, #16 * 9]
    stp x18,    x19,    [sp, #16 * 10]
    stp x20,    x21,    [sp, #16 * 11]
    stp x22,    x23,    [sp, #16 * 12]
    stp x26,    x25,    [sp, #16 * 13]
    stp x28,    x27,    [sp, #16 * 14]

    ret

exit_from_interupt:
    ldr x0,     [sp, #16 * 0]
    add sp, sp, #16

    b exit_from_syscall


enter_from_syscall_internal:
    sub	sp, sp, #16 * 14

    stp x1,     x2,     [sp, #16 * 0]
    stp x3,     x4,     [sp, #16 * 1]
    stp x5,     x6,     [sp, #16 * 2]
    stp x7,     x8,     [sp, #16 * 3]
    stp x9,     x10,    [sp, #16 * 4]
    stp x10,    x11,    [sp, #16 * 5]
    stp x12,    x13,    [sp, #16 * 6]
    stp x14,    x15,    [sp, #16 * 7]
    stp x16,    x17,    [sp, #16 * 8]
    stp x18,    x19,    [sp, #16 * 9]
    stp x20,    x21,    [sp, #16 * 10]
    stp x22,    x23,    [sp, #16 * 11]
    stp x26,    x25,    [sp, #16 * 12]
    stp x28,    x27,    [sp, #16 * 13]

    ret

exit_from_syscall:
    ldp x1,     x2,     [sp, #16 * 0]
    ldp x3,     x4,     [sp, #16 * 1]
    ldp x5,     x6,     [sp, #16 * 2]
    ldp x7,     x8,     [sp, #16 * 3]
    ldp x9,     x10,    [sp, #16 * 4]
    ldp x10,    x11,    [sp, #16 * 5]
    ldp x12,    x13,    [sp, #16 * 6]
    ldp x14,    x15,    [sp, #16 * 7]
    ldp x16,    x17,    [sp, #16 * 8]
    ldp x18,    x19,    [sp, #16 * 9]
    ldp x20,    x21,    [sp, #16 * 10]
    ldp x22,    x23,    [sp, #16 * 11]
    ldp x26,    x25,    [sp, #16 * 12]
    ldp x28,    x27,    [sp, #16 * 13]

    add	sp, sp, #16 * 14

    ldp x30,    x29,    [sp]
    add	sp, sp, #16

    eret


.section .bss
.align 3 
.global user_program_x30
user_program_x30:
.skip 8
