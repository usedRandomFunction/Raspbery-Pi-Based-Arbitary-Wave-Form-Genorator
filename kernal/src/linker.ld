ENTRY(_start)

SECTIONS
{
    . = 0xFFFF000000080000;

    .text.boot : {          /* These are stored here since after boot they are not need to se let the stack overwrite them*/
        KEEP(*(.text.boot))
    }
    .text.reloader : {
        KEEP(*(.text.reloader))
    }

    . = ALIGN(4096);
    .stack (NOLOAD) : {
        boot_pg_dir = .; /* the boot_pg_dir is over written shortly after booting so it can be loaded into the bottom of the stack. */
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
    __end = .;


    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start);