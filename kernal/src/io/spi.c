#include "io/spi.h"

#include "run_time_kernal_config.h"
#include "io/memoryMappedIO.h"
#include "io/propertyTags.h"
#include "lib/clocks.h"
#include "lib/timing.h"
#include "io/printf.h"
#include "io/gpio.h"

#define SPI0_CS_LEN_LONG            (1 << 25)
#define SPI0_CS_DMA_LEN             (1 << 24)
#define SPI0_CS_CSPOL2              (1 << 23)
#define SPI0_CS_CSPOL1              (1 << 22)
#define SPI0_CS_CSPOL0              (1 << 21)
#define SPI0_CS_CSPOL_OFFSET_BEIGN  21
#define SPI0_CS_RXF                 (1 << 20)
#define SPI0_CS_RXR                 (1 << 19)
#define SPI0_CS_TXD                 (1 << 18)
#define SPI0_CS_RXD                 (1 << 17)
#define SPI0_CS_DONE                (1 << 16)
#define SPI0_CS_TE_EN               (1 << 15)
#define SPI0_CS_LMONO               (1 << 14)
#define SPI0_CS_LEN                 (1 << 13)
#define SPI0_CS_REN                 (1 << 12)
#define SPI0_CS_ADCS                (1 << 11)
#define SPI0_CS_INTR                (1 << 10)
#define SPI0_CS_INTD                (1 << 9)
#define SPI0_CS_DMAEN               (1 << 8)
#define SPI0_CS_TA                  (1 << 7)
#define SPI0_CS_CSPOL               (1 << 6)
#define SPI0_CS_CLEAR_RX_FIFO       (1 << 5)
#define SPI0_CS_CLEAR_TX_FIFO       (1 << 4)
#define SPI0_CS_CPOL                (1 << 3)
#define SPI0_CS_CPHA                (1 << 2)
#define SPI0_CS_CS_MASK             (3 << 0)


void initialize_spi0(uint32_t freqency)
{
    if (is_running_in_qemu)
        return;

    for (int i = 7; i <= 11; i++)
        gpio_function_select(i, GPFSEL_Alternate0);

    mmio_write(SPI0_CS, 0);

    spi0_set_clock_rate(freqency);
}

void spi0_set_clock_rate(uint32_t freqency)
{
    if (is_running_in_qemu)
        return;

    uint32_t core_freqency = get_clock_rate(PROPERTY_TAG_CLOCK_ID_CORE);
    uint32_t devider = core_freqency / freqency;
    
    mmio_write(SPI0_CLK, devider);
}

void spi0_set_chip_select_polarity(int cs, bool polarity)
{
    if (cs < 0 || cs > 3 || is_running_in_qemu)       // TODO maby some sort of exception here
        return;

    uint32_t controll = mmio_read(SPI0_CS);

    uint32_t polarity_bit = 1;
    polarity_bit <<= SPI0_CS_CSPOL_OFFSET_BEIGN + cs;

    if (polarity) 
        controll |= polarity_bit;
    else
        controll &= ~polarity_bit;

    mmio_write(SPI0_CS, controll);
}

void spi0_write_read(int cs, const void* send_buffer, void* recive_buffer, size_t n)
{
    const uint8_t* send_buffer_u8 = send_buffer;
    uint8_t* recive_buffer_u8 = recive_buffer;
    
    if (cs < 0 || cs > 3 || is_running_in_qemu)       // TODO maby some sort of exception here
        return;
    
    uint32_t controll = mmio_read(SPI0_CS);

    controll &= ~SPI0_CS_CS_MASK;
    controll |= SPI0_CS_CLEAR_RX_FIFO | SPI0_CS_CLEAR_TX_FIFO | SPI0_CS_TA | SPI0_CS_CPOL | SPI0_CS_CPHA | cs;

    mmio_write(SPI0_CS, controll);

    size_t bytes_written = 0;
    size_t bytes_read = 0;

    while (bytes_written < n || bytes_read < n)
    {
        controll = mmio_read(SPI0_CS);

        if ((controll & SPI0_CS_TXD) && bytes_written < n)
        {
            uint32_t data = 0;

            if (send_buffer)
                data = (uint32_t)*send_buffer_u8++;

            bytes_written++;
            mmio_write(SPI0_FIFO, data);
        }

        if ((controll & SPI0_CS_RXD) && bytes_read < n)
        {
            uint32_t data = 0;

            data = mmio_read(SPI0_FIFO);

            if (recive_buffer_u8)
                *recive_buffer_u8++ = (uint8_t)data;

            bytes_read++;
        }
    }
    
    while ((controll & SPI0_CS_DONE) == 0)
    {
        controll = mmio_read(SPI0_CS);
    }
    
    mmio_write_bitwise_and(SPI0_CS, ~SPI0_CS_TA);
}

void spi0_write(int cs, const void* buffer, size_t n)
{
    spi0_write_read(cs, buffer, NULL, n);
}

void spi0_read(int cs, void* buffer, size_t n)
{
    spi0_write_read(cs, NULL, buffer, n);
}