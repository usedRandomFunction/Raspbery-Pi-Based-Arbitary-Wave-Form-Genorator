#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Initializes SPI0 for standard mode
// Sets alternate pin use for gpio 7 to 11
// @param freqency Will set clock devider to best mach the given freqency 
void initialize_spi0(uint32_t freqency);

// Sets the clock devider to best match the given freqnecy
// @param freqency The freqency to set
void spi0_set_clock_rate(uint32_t freqency);

// Sets the polarity of a given chip select pin
// @param cs The chip select to change [0, 2] inclusive
// @param polarity False is active low, True is active high
void spi0_set_chip_select_polarity(int cs, bool polarity);

// Simultaneously writes and reads the SPI
// @param cs The chip select line to enable
// @param send_buffer The buffer containing the infomation to send
// @param recive_buffer The buffer to store the incomming infomation
// @param n The number of bytes to read / write 
void spi0_write_read(int cs, const void* send_buffer, void* recive_buffer, size_t n);

// Writes the given buffer to the spi
// @param cs The chip select line to enable
// @param buffer The buffer to write to the spi
// @param n The size of the buffer in bytes
void spi0_write(int cs, const void* buffer, size_t n);

// Reads n bytes from the SPI
// @param cs The chip select line to enable
// @param buffer The buffer to write into
// @param n The number of bytes to read
// @note This will still transmit, it will just send zeros
void spi0_read(int cs, void* buffer, size_t n);

#endif