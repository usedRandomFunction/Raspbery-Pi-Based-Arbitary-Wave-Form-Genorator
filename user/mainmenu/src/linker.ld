ENTRY(entry)

SECTIONS
{
    . = 0x40000000;

    .text : 
    { 
        bin/obj/entry.c.o (.text)           /* This is to force entry to be at a specific address */
        *(.text .text.* .gnu.linkonce.t*) 
    }

    . = ALIGN(4096);
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    
    /* Since this is a monolithic read only app it is not possible to have a data section */
    /* . = ALIGN(4096); */
    /* .data : { *(.data .data.* .gnu.linkonce.d*) } */

    . = ALIGN(0x40000000);  /* BSS must be 4 GiB alligned so it can be its own section in memory */
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}