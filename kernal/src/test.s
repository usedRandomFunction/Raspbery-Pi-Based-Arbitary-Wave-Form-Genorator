.section .text
.global test_el0
test_el0:
    mov x0, #0x00
    msr spsr_el1, x0

    adr x0, el0_entry
    msr elr_el1, x0

    eret

el0_entry:
    adrp    x0, .str
    add     x0, x0, :lo12:.str
    mov x8, 0
    svc #0


.section .rodata
.str:
    .string "string\0"
    