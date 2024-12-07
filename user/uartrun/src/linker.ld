ENTRY(_entry)

SECTIONS
{
    . = 0x400000000;
    
    .text : 
    { 
        __text_start = .;
        *(.start)
        *(.text .text.* .gnu.linkonce.t*) 
    }

    . = ALIGN(4096);
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    
    . = ALIGN(0x40000000);  /* BSS must be 4 GiB alligned so it can be its own section in memory */
    .bss (NOLOAD) : {      
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}