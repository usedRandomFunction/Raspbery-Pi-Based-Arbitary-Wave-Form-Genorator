#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdbool.h>
#include <stdint.h>

#include "lib/memory.h"
#include "io/printf.h"


#ifdef __cplusplus
extern "C" {
#endif

// Emptys the mailbox
void mailbox_flush();

// Reads from the given mailbox channel
// @param channel The chanel to read from
// @return The value form the channel
uint32_t mailbox_read(uint8_t channel);

// Writes to the given mailbox channel
// @param data The value to write to the channel
// @param channel The chanel to write to
void mailbox_write(uint32_t data, uint8_t channel);

// Writes then reads from the given mailbox channel
// @param data The value to write to the channel
// @param channel The chanel to write and read from
// @return The value form the channel
uint32_t mailbox_write_read(uint32_t data, uint8_t channel);

// Writes to the given mailbox channel
// @param ptr The virutal address to write to the mailbox
// @param channel The chanel to write to
inline void mailbox_write_alliged_address(void* ptr, uint8_t channel);

// Writes then reads to the given mailbox channel
// @param ptr The virutal address to write to the mailbox
// @param channel The chanel to write and read from
// @return The value form the channel
inline uint32_t mailbox_write_read_alliged_address(void* ptr, uint8_t channel);

// Writes to the given mailbox channel
// @param ptr The physcial address to write to the mailbox
// @param channel The chanel to write to
inline void mailbox_write_alliged_physcial_address(void* ptr, uint8_t channel);

// Writes then reads to the given mailbox channel
// @param ptr The physcial address to write to the mailbox
// @param channel The chanel to write and read from
// @return The value form the channel
inline uint32_t mailbox_write_read_physcial_alliged_address(void* ptr, uint8_t channel);


inline void mailbox_write_alliged_address(void* ptr, uint8_t channel)
{
    mailbox_write_alliged_physcial_address(get_physical_address(ptr), channel);
}

inline uint32_t mailbox_write_read_alliged_address(void* ptr, uint8_t channel)
{
    return mailbox_write_read_physcial_alliged_address(get_physical_address(ptr), channel);
}

inline void mailbox_write_alliged_physcial_address(void* ptr, uint8_t channel)
{
    #ifdef AARCH64
    if (((uint64_t)ptr & 0x0000FFFF0000000F) != 0)
    #else
    if (((uint32_t)ptr & 0x0000000F) != 0)
    #endif
    {
        printf("Failed to write to mailbox, address must fit into a 32 bit integer and be alligned to 16 bytes");
        return;
    }

    size_t ptr_as_int = (size_t)ptr; // To make gcc happy

    mailbox_write((uint32_t)ptr_as_int, channel);
}

inline uint32_t mailbox_write_read_physcial_alliged_address(void* ptr, uint8_t channel)
{
    #ifdef AARCH64
    if (((uint64_t)ptr & 0xFFFFFFFF0000000F) != 0)
    #else
    if (((uint32_t)ptr & 0x0000000F) != 0)
    #endif
    {
        printf("Failed to read from mailbox, address must fit into a 32 bit integer and be alligned to 16 bytes");
        return 0;
    }

    size_t ptr_as_int = (size_t)ptr; // To make gcc happy

    
    return mailbox_write_read((uint32_t)ptr_as_int, channel);
}

#ifdef __cplusplus
}
#endif

#endif