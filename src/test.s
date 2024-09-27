.global test_el0
test_el0:
    mov x0, #0x00
    msr spsr_el1, x0

    adr x0, el0_entry
    msr elr_el1, x0

    eret

el0_entry:
    mov x8, 1
    svc #0
    