ENTRY(_start)

SECTIONS
{
    . = 0x200000;

    .stack (NOLOAD) : {
        . = . + 32K;
    }
    _stack_top = .;

    . = ALIGN(2097152);
    .text : { *(.text .text.* .gnu.linkonce.t*) }

    . = ALIGN(2097152);
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    
    . = ALIGN(2097152);
    .data : { *(.data .data.* .gnu.linkonce.d*) }

    . = ALIGN(2097152);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}