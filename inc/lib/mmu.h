#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void write_page_descriptor(uint64_t* descriptor_address, void* pointer_address, uint16_t upper_attributes, uint16_t lower_attributes, bool block_table_bit);

#ifdef __cplusplus
}
#endif

#endif