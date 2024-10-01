ENTRY(main)

SECTIONS
{
    . = 0x40000000;

    .text : { *(.text .text.* .gnu.linkonce.t*) }

    . = ALIGN(0x40000000);
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    
    . = ALIGN(0x40000000);
    .data : { *(.data .data.* .gnu.linkonce.d*) }

    . = ALIGN(0x40000000);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}