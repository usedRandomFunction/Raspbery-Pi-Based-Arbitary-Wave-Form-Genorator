#if USE_MINI_UART == 1

#include "io/memoryMappedIO.h"
#include "lib/timing.h"
#include "io/gpio.h"
#include "io/uart.h"

void uart_init(int baudrate)
{
    // Set Function register
	gpio_function_select(14, GPFSEL_Alternate5);
	gpio_function_select(15, GPFSEL_Alternate5);

    // Dissable GPIO pull up / down
    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    mmio_write(GPPUDCLK0, 0);

    mmio_write(AUX_ENABLES, 1);                                     //Enable mini uart (this also enables access to its registers)
    mmio_write(AUX_MU_CNTL_REG, 0);                                 //Disable auto flow control and disable receiver and transmitter (for now)
    mmio_write(AUX_MU_IER_REG, 0);                                  //Disable receive and transmit interrupts
    mmio_write(AUX_MU_LCR_REG, 3);                                  //Enable 8 bit mode
    mmio_write(AUX_MU_MCR_REG, 0);                                  //Set RTS line to be always high
    mmio_write(AUX_MU_BAUD_REG, 250000000 / (8 * baudrate) - 1);    //Set baud rate
    mmio_write(AUX_MU_CNTL_REG, 3);                                 //Finally, enable transmitter and receiver
}


void uart_putc(unsigned char c)
{
    while(1) // wait for Transmitter idle
        if(mmio_read(AUX_MU_LSR_REG) & 0x20) 
            break;

    mmio_write(AUX_MU_IO_REG, c);
}


char uart_getc()
{
    while(1) // wait for reciver overrun
        if (mmio_read(AUX_MU_LSR_REG) & 0x01) 
            break;

    return mmio_read(AUX_MU_IO_REG) & 0xFF;
}

#endif