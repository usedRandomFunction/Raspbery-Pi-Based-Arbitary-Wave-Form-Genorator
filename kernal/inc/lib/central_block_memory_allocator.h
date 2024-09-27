#ifndef CENTRAL_BLOCK_MEMORY_ALLOCATOR_H
#define CENTRAL_BLOCK_MEMORY_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// The memory space is divided into two regions, controll and allocation
// The allocation region is devided in to blocks of 2^block_size_as_power_of_two bytes
// One block is the smallest allocation size
// 
// The controll reigon is used a long aray of two-bit numbers as follows
// +=========+=====+=====+=====+=====+==========+
// | Address | 0-1 | 2-3 | 4-5 | ... | 2n-(2n+1)|
// +---------+-----+-----+-----+-----+----------+
// |   Use   | B0  | B1  | B2  | ... |    Bn    |
// +=========+=====+=====+=====+=====+==========+
//
// Where Bn is the block control flag of the nTh block,
// Where the values are as follows
// +=======+=======================+
// | Value |        Meaning        |
// +-------+-----------------------+
// |   0   |      Unallocated      |
// +-------+-----------------------+
// |   1   |  Start of allocation  |
// +-------+-----------------------+
// |   2   | Middile of allocation |
// +-------+-----------------------+
// |   3   |   End of allocation   |
// +=======+=======================+


struct central_block_memory_allocator_header
{
    void* controll_region_start;
    void* allocation_region_start;
    size_t block_size_as_power_of_two;
    size_t number_of_alocated_blocks;
    size_t number_of_total_blocks;
    size_t number_of_free_blocks;
};

typedef struct  central_block_memory_allocator_header central_block_memory_allocator_header;

// Creates a 'central block memory allocator' [see central_block_memory_allocator] 
// If the header is to 'live' inside its block, just add the size of the header to mem_start
// and remove it from region_size, note that the controll blocks will 'live' inside
// the memory region previded
// @param mem_start The address that the allocato will use
// @param region_size The size in bytes of the region of memory it controll
// @param block_size_as_power_of_two The size of the blocks as a power of two
// @param header The allocator header
void initialize_central_block_memory_allocator(void* mem_start, size_t region_size, size_t block_size_as_power_of_two, central_block_memory_allocator_header* header);

// notes a allocaton as 'free' not much more to say
// @param ptr The pointer to free
// @param header The allocator header
void central_block_memory_allocator_free(void* ptr, central_block_memory_allocator_header* header);

// Finds a block of memory with in the allocator that is free and returns its address,
// and subsecently marks it as allocated
// (Identical to central_block_memory_allocator_alloc_alligned(size, 0, header);)
// @param size the size of the allocation
// @param header The allocator header
// @return The pointer to the allocation or NULL if failed
inline void* central_block_memory_allocator_alloc(size_t size, central_block_memory_allocator_header* header);

// Finds a block of memory with in the allocator that is free and returns its address,
// and subsecently marks it as allocated
// if allignment == 0 then it will return an address with no defined allignment, and will likly will default to block_size
// @param size the size of the allocation
// @param allignment_as_power_of_two The required allignment as a power of two
// @param header The allocator header
// @return The pointer to the allocation or NULL if failed
void* central_block_memory_allocator_alloc_alligned(size_t size, size_t allignment_as_power_of_two, central_block_memory_allocator_header* header);

enum
{
    CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE = 0,
    CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_START = 1,
    CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_MIDDLE = 2,
    CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_END = 3,
};


inline void* central_block_memory_allocator_alloc(size_t size, central_block_memory_allocator_header* header)
{
    return central_block_memory_allocator_alloc_alligned(size, 0, header);
}



#ifdef __cplusplus
}
#endif
#endif