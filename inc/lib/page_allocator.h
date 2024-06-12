// The page allocator only keeps track of what pags in memory are used, 
// it does not mess with the translation table

#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct page_allocation_info
{
    // The offset in pages from page zero
    uint32_t first_page;

    // The size in pages
    uint32_t size;

    // A pointer to a the next section if it exists
    // NULL if it does not exist, dosn't have to be continuous
    struct page_allocation_info* next;
};

typedef struct page_allocation_info page_allocation_info;

const extern size_t page_allocator_page_size_as_power_of_two;
const extern size_t page_allocator_page_size_bytes;

// Initializes the page allocator, must be called after the heap is set up
// @return True is success, False if failer
bool initialize_page_allocator();

// Creates a new allocation
// @param size: Size of allocation in bytes
// @return Pointer to allocation header or NULL if failed
page_allocation_info* create_new_page_allocation(size_t size);

// Creates a new continuous allocation at the given phyiscal address
// @param physical_address_start: The physica address that this allocation would begin at
// @param size: Size of allocation in bytes
// @return Pointer to allocation header or NULL if failed
page_allocation_info* create_new_page_allocation_at_continuous_physical_address(void* physical_address_start, size_t size);

// Finds the size of a given page allocation
// @param allocation A pointer to the allocation to messure
// @return The size of the allocation in bytes
size_t get_page_allocation_size(page_allocation_info* allocation);

// Resizes the given allocation by contain new_size rounded up to the nearest block.
// It will shink the allocation if needed
// @param allocation A pointer to the allocation to messure
// @param new_size The new target size (in bytes) for the allocation
// @return True if success False if failer
bool resize_page_allocation(page_allocation_info* allocation, size_t new_size);

// Marks an entire allocation as free and deletes all traces of its existance
// @param allocation A pointer to the allocation to messure
void destroy_page_allocation(page_allocation_info* allocation);

// Cacluates the total number of allocated pages
// @return Number of allocated pages
uint32_t get_total_allocated_pages();

// Cacluates the total number of free pages
// @return Number of free pages
uint32_t get_total_free_pages();

#ifdef __cplusplus
}
#endif

#endif