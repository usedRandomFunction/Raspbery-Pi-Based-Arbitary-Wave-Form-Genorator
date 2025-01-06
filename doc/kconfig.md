# List Of Possible C Deffinitions


| Name | Description |
|------|-------------|
| USE_MINI_UART | When USE_MINI_UART = 1, the mini uart will be used,<br>When USE_MINI_UART = 0, the pl011 uart is used.
| UART_BAUD_RATE | Sets the baurd rate used by kernal.c.<br>When the value is unset, it defults to 115200
| UART_BASE_CLOCK_FREQUENCY | Sets the base freqency of the PL011 Uart
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_UART_PUT_IF_FAILED_ALLOC | If defined a central_block_memory_allocator<br>will print to uart if alloc failed
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_UART_PUT_IF_FAILED_FREE | If defined a central_block_memory_allocator<br>will print to uart if free failed
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_UART_PUT_ALLOC_FREE| If defined a central_block_memory_allocator<br>will print info about new allocations and freeing (use for debuging)
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_KERNEL_PANIC_FAILED | If defined a central_block_memory_allocator<br>will trigger a kernel panic if it fails
| PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO | Sets the size of pages to be used by the page allocator 
| UART_BASE_CLOCK_FREQUENCY | Sets the base clock freqency of the UART
| ALLWAYS_SHIRNK_FRAME_BUFFER_IF_POSSIBLE | States wherever or not, the frame buffer should shink if possible<br>applys when reducing the number of virtual buffers<br><br>Note: This value is also included system.cfg, this header value only<br>controlls the defults, and is also used before the config is loaded,<br>since the frame buffer is initialized before the config file is loaded.
| MAXIMUM_NUMBER_OF_FRAME_BUFFERS | Stores the maximum allowed number of virtual frame buffers<br><br>Note: This value is also included system.cfg, this header value only<br>controlls the defults, and is also used before the config is loaded,<br>since the frame buffer is initialized before the config file is loaded.
| MINIMUM_NUMBER_OF_FRAME_BUFFERS | Stores the minimum allowed number of virtual frame buffers<br><br>Note: This value is also included system.cfg, this header value only<br>controlls the defults, and is also used before the config is loaded,<br>since the frame buffer is initialized before the config file is loaded.
| DISPLAY_HEIGHT | Stores the height of the display in pixels<br><br>Note: This value is also included system.cfg, this header value only<br>controlls the defults, and is also used before the config is loaded,<br>since the frame buffer is initialized before the config file is loaded.
| DISPLAY_WIDTH | Stores the width of the display in pixels<br><br>Note: This value is also included system.cfg, this header value only<br>controlls the defults, and is also used before the config is loaded,<br>since the frame buffer is initialized before the config file is loaded.
| CONSOLE_MAX_HEIGHT | Max y valye of the console (in pixels)
| SD_VERBOSE_LOGGING | Enable verbose logging for the SD functions if defined
| MMIO_VIRUTAL_ADDRESS_BASE | Sets the base virtual addreess for the MMIO (must be a void*)
| FRAMEBUFFER_VIRUTAL_ADDRESS_BASE | Sets the base virtual addreess for the framebuffer (must be a void*)
| INTERUPT_END_EVENT_MAX_HANDLERS | Sets the maximum number of handlers for the `interupt_end` event
| MAX_NUMBER_OF_USER_INTERUPT_HANDLERS | Sets the maximum number of handlers avaible to the user interupt functions