# List Of Possible C Deffinitions


| Name | Description |
|------|-------------|
| USE_MINI_UART | When USE_MINI_UART = 1, the mini uart will be used,
||                When USE_MINI_UART = 0, the pl011 uart is used.
| UART_BAUD_RATE | Sets the baurd rate used by kernal.c.
||                 When the value is unset, it defults to 115200
| UART_BASE_CLOCK_FREQUENCY | Sets the base freqency of the PL011 Uart
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_UART_PUT_IF_FAILED_ALLOC | If defined a central_block_memory_allocator
|| will print to uart if alloc failed
| CENTRAL_BLOCK_MEMORY_ALLOCATOR_UART_PUT_IF_FAILED_FREE | If defined a central_block_memory_allocator
|| will print to uart if free failed
| PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO | Sets the size of pages to be used by the page allocator 