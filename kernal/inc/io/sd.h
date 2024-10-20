#ifndef SD_H
#define SD_H

#include <stdint.h>

enum 
{
    SD_OK = 0,
    SD_TIMEOUT = -1,
    SD_ERR = -2,
};

// Initializes the emmc for the sd card
// @return SD_OK if success
int initialize_sd();

// Reads blocks form the SD card
// @param lba The locigal block address to read
// @param buf The memory to read into
// @param num The nubmer of blocks to read
// @return Number of bytes read, 0 on failer
int sd_readblock(uint32_t lba, void* buf, uint32_t num);

// Writes blocks to the SD card
// @param lba The locigal block address to start writing at
// @param buf The memory to write from
// @param num The nubmer of blocks to write
// @return Number of bytes written, 0 on failer
int sd_writeblock(uint32_t lba, const void* buf, uint32_t num);

// Reads section_size sectors copys num bytes from buf at offset, and then writes it back to the SD
// @param lba The locigal block address to start the read / write at
// @param buf The memory to copy
// @param offset The offset from the start of the section (In bytes)
// @param num The number of bytes to copy from buf
// @param section_size The number of sectors the "section" contains
// @param section (Optional) A pointer to the working buffer for this function, if NULL it will be allocted automaticly
// @return 1 On success, 0 on failer
int sd_write_section(uint32_t lba, void* buf, uint32_t offset, uint32_t num, uint32_t section_size, void* section);

#endif