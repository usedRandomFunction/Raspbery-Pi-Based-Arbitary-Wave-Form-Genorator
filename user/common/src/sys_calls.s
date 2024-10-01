.section .text

.global sys_call_print
sys_call_print:
    mov w8, 0
    svc #0
    ret

.global sys_call_exit
sys_call_exit:
    mov w8, 1
    svc #0