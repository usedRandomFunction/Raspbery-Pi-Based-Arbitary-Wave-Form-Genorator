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
void write_page_descriptor(uint64_t* descriptor_address, void* pointer_address, uint64_t attributes, bool block_table_bit);

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
    MMU_ATTRIBUTES_EL0_ACCESS = 0b01 << 6, // in ARM Architecture Reference Manual - ARMv8 p.g 2158
    MMU_ATTRIBUTES_READ_ONLY = 0b10 << 6,
    MMU_ATTRIBUTES_ACCESS_BIT = 0b1 << 10,
    MMU_ATTRIBUTES_nGnRnE = 0 << 2,                       // Non-Gathering, Non-Reordering, No Early write acknowledgement
    MMU_ATTRIBUTES_NON_CACHABLE = 1 << 2,                 // Non-cacheable attribute
    MMU_ATTRIBUTES_CACHABLE = 2 << 2,                     // Cacheable read write

    MMU_ATTRIBUTES_PRIVILEGED_EXECUTE_NEVER = 1ULL << 53,    // Privileged Execute Never
    MMU_ATTRIBUTES_EXECUTE_NEVER  = 1ULL << 54               // Execute Never
};

#ifdef __cplusplus
}
#endif

#endif