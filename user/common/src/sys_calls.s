.section .text

.global set_abi_version
set_abi_version:
    mov w8, 0
    svc #0
    ret

.global exit
exit:
    mov w8, 1
    svc #0

.global printf
printf:
    mov w8, 2
    svc #0
    ret

.global putchar
putchar:
    mov w8, 3
    svc #0
    ret

.global uart_putc
uart_putc:
    mov w8, 4
    svc #0
    ret

.global uart_getc
uart_getc:
    mov w8, 5
    svc #0
    ret

.global uart_poll
uart_poll:
    mov w8, 6
    svc #0
    ret

.global open
open:
    mov w8, 7
    svc #0
    ret

.global close
close:
    mov w8, 8
    svc #0
    ret

.global get_file_size
get_file_size:
    mov w8, 9
    svc #0
    ret

.global read
read:
    mov w8, 10
    svc #0
    ret

.global write
write:
    mov w8, 11
    svc #0
    ret

.global lseek
lseek:
    mov w8, 12
    svc #0
    ret

.global truncate
truncate:
    mov w8, 13
    svc #0
    ret

.global ftruncate
ftruncate:
    mov w8, 14
    svc #0
    ret

.global remove
remove:
    mov w8, 15
    svc #0
    ret

.global fremove
fremove:
    mov w8, 16
    svc #0
    ret
