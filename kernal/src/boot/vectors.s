.section ".text"

.global _vectors

.macro vector_table_entry function
.align 7
b \function
.endm

.macro exception_handler_kernal_error id
.align  7
mov x0, \id
b       arm_exception_handler
.endm

// TODO impement user error handler
.macro exception_handler_user_error id
.align  7
mov x0, \id
b       arm_exception_handler
.endm

.align	11
_vectors:
    // Exceptions for EL1t
    exception_handler_kernal_error #0 // synchronous
    exception_handler_kernal_error #1 // IRQ
    exception_handler_kernal_error #2 // FIQ
    exception_handler_kernal_error #3 // SError

    // Exceptions for EL1h
    exception_handler_kernal_error #0 // synchronous
    exception_handler_kernal_error #1 // IRQ
    exception_handler_kernal_error #2 // FIQ
    exception_handler_kernal_error #3 // SError

    // Exceptions for EL0 (AArch64)
    vector_table_entry .el0_svn // synchronous
    exception_handler_user_error #1 // IRQ
    exception_handler_user_error #2 // FIQ
    exception_handler_user_error #3 // SError

    // Exceptions for EL0 (AArch32)
    exception_handler_user_error #0 // synchronous
    exception_handler_user_error #1 // IRQ
    exception_handler_user_error #2 // FIQ
    exception_handler_user_error #3 // SError

.el0_svn:
    bl enter_from_syscall
    mrs	x24, esr_el1				// read the syndrome register
	lsr	x24, x24, #26		// exception class
	cmp	x24, #0x15			// SVC in 64-bit state
	b.eq	.el0_svc
    mov x0, 0
    b       arm_exception_handler // TODO USER ERROR FUNCTION

.el0_svc:
    cmp     x8, 2 // Number of syscalls
    bge     .el0_svc_failed
    ldr     x9, =system_call_table
    lsl     x8, x8, #3 // Sames as x8 = x8 * 8 (2^3)
    add     x9, x9, x8
    ldr     x10,  [x9]
    blr     x10
    bl exit_syscall


.el0_svc_failed:
    mov x0, 5
    b       arm_exception_handler // TODO USER ERROR FUNCTION

enter_from_syscall:
    sub	sp, sp, #16 * 15
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
    ret

exit_syscall:
    ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
    add	sp, sp, #16 * 15	

    eret
