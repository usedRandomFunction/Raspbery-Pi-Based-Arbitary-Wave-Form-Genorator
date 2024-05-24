ENTRY(_start)

SECTIONS
{
    . = 0xFFFF000000000000;

    .text.boot : {
        KEEP(*(.text.boot))
    }

    .stack (NOLOAD) : {
        . = ALIGN(16);
        . = . + 32K;
    }
    _stack_top = .;

    .text : { *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

    . = ALIGN(4096); /* Is after end as this is only needed in the boot process and can be overwritten latter */
	pg_dir = .;

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start);