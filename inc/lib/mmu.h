#ifndef MMU_H
#define MMU_H

#include <stdbool.h>
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

// Writes to the ttbr1_el1 register
// @param ptr thr new page table address (must be physcial address)
void set_ttbr1_el1(void* ptr);

// Writes to the ttbr0_el1 register
// @param ptr thr new page table address (must be physcial address)
void set_ttbr0_el1(void* ptr);

// Invalidates the translation lookaside buffer
void invalidate_tlb();

// Enables both the data and instruction caches
void enable_caches();

// Dissables both the data and instruction caches
void dissable_caches();

// Invalidates both the data and instruction caches
void invalidate_caches();

enum
{
    MMU_LOWER_ATTRIBUTES_ACCESS_BIT = 0b1<< 8,
    MMU_LOWER_ATTRIBUTES_nGnRnE = 0,                            // Non-Gathering, Non-Reordering, No Early write acknowledgement
    MMU_LOWER_ATTRIBUTES_NON_CACHABLE = 1,                      // Non-cacheable attribute
    MMU_LOWER_ATTRIBUTES_CACHABLE_READ_WRITE_EXECTUE = 2,       // Cacheable read write exicute
    MMU_LOWER_ATTRIBUTES_CACHABLE_READ_WRITE_NON_EXECTUE = 3,   // Cacheable read write exicute
};

#ifdef __cplusplus
}
#endif

#endif