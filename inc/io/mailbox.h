#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdbool.h>
#include <stdint.h>

#include "uart.h"


#ifdef __cplusplus
extern "C" {
#endif

void mailbox_flush();
uint32_t mailbox_read(uint8_t channel);
void mailbox_write(uint32_t data, uint8_t channel);
uint32_t mailbox_write_read(uint32_t data, uint8_t channel);

inline void mailbox_write_alligedAddress(void* ptr, uint8_t channel)
{
    #ifdef AARCH64
    if (*(uint64_t*)&ptr & 0xFFFFFFFF0000000F != 0)
    #else
    if (*(uint32_t*)&ptr & 0x0000000F != 0)
    #endif
    {
        uart_puts("Failed to write to mailbox, address must fit into a 32 bit integer and be alligned to 16 bytes");
        return;
    }

    mailbox_write(*(uint32_t*)&ptr, channel);
}

inline uint32_t mailbox_write_read_alligedAddress(void* ptr, uint8_t channel)
{
    #ifdef AARCH64
    if (*(uint64_t*)&ptr & 0xFFFFFFFF0000000F != 0)
    #else
    if (*(uint32_t*)&ptr & 0x0000000F != 0)
    #endif
    {
        uart_puts("Failed to read from mailbox, address must fit into a 32 bit integer and be alligned to 16 bytes");
        return 0;
    }
    
    return mailbox_write_read(*(uint32_t*)&ptr, channel);
}

#ifdef __cplusplus
}
#endif

#endif