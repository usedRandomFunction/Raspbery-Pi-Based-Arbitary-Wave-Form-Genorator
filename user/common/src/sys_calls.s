.section .text

// program management

.global set_abi_version
set_abi_version:
    mov w8, 0
    svc #0
    ret

.global exit
exit:
    mov w8, 1
    svc #0

.global vmemmap
vmemmap:
    mov w8, 2
    svc #0
    ret

// basic IO

.global printf
printf:
    mov w8, 0
    svc #1
    ret

.global putchar
putchar:
    mov w8, 1
    svc #1
    ret

.global uart_putc
uart_putc:
    mov w8, 2
    svc #1
    ret

.global uart_getc
uart_getc:
    mov w8, 3
    svc #1
    ret

.global uart_poll
uart_poll:
    mov w8, 4
    svc #1
    ret

// File IO

.global open
open:
    mov w8, 0
    svc #2
    ret

.global close
close:
    mov w8, 1
    svc #2
    ret

.global get_file_size
get_file_size:
    mov w8, 2
    svc #2
    ret

.global read
read:
    mov w8, 3
    svc #2
    ret

.global write
write:
    mov w8, 4
    svc #2
    ret

.global lseek
lseek:
    mov w8, 5
    svc #2
    ret

.global truncate
truncate:
    mov w8, 6
    svc #2
    ret

.global ftruncate
ftruncate:
    mov w8, 7
    svc #2
    ret

.global remove
remove:
    mov w8, 8
    svc #2
    ret

.global fremove
fremove:
    mov w8, 9
    svc #2
    ret
