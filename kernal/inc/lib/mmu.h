#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Writes a page table entry
// @param descriptor_address A pointer to the address to write to
// @param pointer_address The physical address the entry points to
// @param upper_attributes Attributes for the descriptor
// @param lower_attributes Attributes for the descriptor
// @param block_table_bit If set to true entry points to other table entry if false entry points to page address
void write_page_descriptor(uint64_t* descriptor_address, void* pointer_address, uint16_t upper_attributes, uint16_t lower_attributes, bool block_table_bit);

// Prints the given descriptor to the UART
void print_page_descriptor(uint64_t* descriptor_address);

// Writes to the ttbr1_el1 register
// @param ptr thr new page table address
void set_ttbr1_el1(void* ptr);

// Writes to the ttbr0_el1 register
// @param ptr thr new page table address
void set_ttbr0_el1(void* ptr);

// Invalidates the translation lookaside buffer
void invalidate_tlb();

// Enables both the data and instruction caches
void enable_caches();

// Dissables both the data and instruction caches
void dissable_caches();

// Invalidates data cache with in the given range
// @param VA_start The virtual address to start with
// @param VA_end The virtual address to end with
void invalidate_data_cache(void* VA_start, void* VA_end);

// Invalidates data cache with in the given range
// @param VA_start The virtual address to start with
// @param size The nubmer of bytes to invalide
void invalidate_data_cache_of_size(void* VA_start, size_t size);

enum
{
    MMU_LOWER_ATTRIBUTES_ACCESS_BIT = 0b1<< 8,
    MMU_LOWER_ATTRIBUTES_nGnRnE = 0,                            // Non-Gathering, Non-Reordering, No Early write acknowledgement
    MMU_LOWER_ATTRIBUTES_NON_CACHABLE = 1,                      // Non-cacheable attribute
    MMU_LOWER_ATTRIBUTES_CACHABLE = 2,                          // Cacheable read write

    MMU_UPPER_ATTRIBUTES_PRIVILEGED_EXECUTE_NEVER = 1 << 5,     // Privileged Execute Never
    MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER  = 1 << 6                // Execute Never
};

#ifdef __cplusplus
}
#endif

#endif