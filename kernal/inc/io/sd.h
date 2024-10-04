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



#endif