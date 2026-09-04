ENTRY(_entry)

SECTIONS
{
    . = 0x40000000;
    
    .text : 
    { 
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

    . = ALIGN(0x40000000);  /* Heap must be 4 GiB alligned so it can be its own section in memory */
    .heap (NOLOAD) : {
        __heap_start = .;
        . = . + 128K;       /* Note: this defines the minimum size of the heap vmemmap, may provide a bigger biffer */
                            /* in that case, _entry will allow the heap to use the extra space */
        __heap_end = .;
    }

    /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }

}