#include "lib/central_block_memory_allocator.h"
#include "lib/arm_exceptions.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "io/printf.h"


static uint_fast8_t get_nth_block_controll(uint_fast8_t* controll_region_start, size_t n);
static void write_nth_block_controll(uint_fast8_t* controll_region_start, uint_fast8_t data, size_t n);

// TODO don't include the size of the controll block while calculating number of blocks

void initialize_central_block_memory_allocator(void* mem_start, size_t region_size, size_t block_size_as_power_of_two, central_block_memory_allocator_header* header)
{
    header->block_size_as_power_of_two = block_size_as_power_of_two;
    header->controll_region_start = mem_start; // The controll region will allways be the first thing in the memory region
    
    const size_t block_size = 1 << block_size_as_power_of_two;

    size_t total_number_of_blocks = region_size >> block_size_as_power_of_two;
    // While memory could be saved be not including the controll region... that will take to long for now
    size_t controll_region_size_bytes = total_number_of_blocks >> 2;
    // this is the same as total_number_of_blocks / (2^2)

    // Now for some  bitwise fuckery
    // +=============+=======+             
    // |             | ←-n-→ |
    // |xxxxxxxxxxxx | xxxxx |
    // +=============+=======+
    // When we shift right be n we remove the first n bits
    // This has the effect of rounding down; we do not want that
    // So if any of the first n bits are on we add on 1 to the counter
    // becouse (2^n) -1 is a number with the first n bits on the code looks as follows
    if (total_number_of_blocks & ((1 << 2) - 1))
        controll_region_size_bytes++;


    size_t controll_region_size_blocks = controll_region_size_bytes >> block_size_as_power_of_two;
    if (controll_region_size_bytes & (total_number_of_blocks - 1))
        controll_region_size_blocks++;

    // The start of the alloc region should be alligned to block_size
    void* alloc_region_start = void_ptr_offset_bytes(mem_start, controll_region_size_bytes);
    size_t alloc_region_start_alignment_error = (size_t)alloc_region_start & (block_size - 1);

    if (alloc_region_start_alignment_error != 0)
    {
        // This is how much we need to add
        alloc_region_start_alignment_error = block_size - alloc_region_start_alignment_error;
        alloc_region_start = void_ptr_offset_bytes(alloc_region_start, alloc_region_start_alignment_error);

        // If this means that we lost a block so be it, but we dont need to note that
        size_t controll_region_space_left_to_block_offset = 
            (controll_region_size_blocks << block_size_as_power_of_two) - controll_region_size_bytes;

        if (controll_region_space_left_to_block_offset < alloc_region_start_alignment_error)
            total_number_of_blocks--;
    }

    
    total_number_of_blocks -= controll_region_size_blocks;
    header->number_of_total_blocks = total_number_of_blocks;
    header->number_of_free_blocks = total_number_of_blocks;
    header->number_of_alocated_blocks = 0;
    header->allocation_region_start = alloc_region_start;

    // Now all the values are set zero the controll region to get every thing ready
    memclr(header->controll_region_start, controll_region_size_bytes);
    
    const int prefix_message_size = 34;
    char prefix[prefix_message_size + 8 + 3]; 
    strcpy_s("central_block_memory_allocator_0x", prefix_message_size + 8 + 3, &prefix[0]);
    hex_size_t((size_t)header, prefix + prefix_message_size - 1, 8);
    strcat_s(prefix, prefix_message_size + 8 + 3, ": ");

    printf("%sAllocator Started\n", prefix);

    printf("%sControll Region uses: %d bytes\n", prefix, controll_region_size_blocks << block_size_as_power_of_two);

    printf("%sAllocatable Space: %d Kib of %d Kib\n",
        prefix,
        (header->number_of_total_blocks << block_size_as_power_of_two) / 1024,
        region_size/ 1024);


    printf("%sTotal Blocks: %d\n", prefix, header->number_of_total_blocks);

    // All set up
    // 𝕋𝕙𝕖𝕪'𝕣𝕖 𝕨𝕒𝕚𝕥𝕚𝕟𝕘 𝕗𝕠𝕣 𝕪𝕠𝕦 𝕘𝕠𝕣𝕕𝕠𝕟...𝕚𝕟 𝕥𝕙𝕖 𝕥𝕖𝕤𝕥 𝕔𝕙𝕒𝕞𝕓𝕖𝕣𝕣𝕣𝕣𝕣𝕣
}

void central_block_memory_allocator_free(void* ptr, central_block_memory_allocator_header* header)
{
    #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_ALLOC_FREE

    printf("\ncentral_block_memory_allocator_%x: Freeing %x\n", header, (size_t)ptr);
    #endif

    const void* block_offset = (char*)ptr - (size_t)header->allocation_region_start;
    size_t blockID = (size_t)(block_offset) >> header->block_size_as_power_of_two;
    // Dives the pointer by 2^block_size_as_power_of_two (rounds down)

    if (blockID >= header->number_of_total_blocks) // Is block does not exist, do free it
    {
        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_IF_FAILED_FREE
        printf("\ncentral_block_memory_allocator_%x: Failed to free block %x, block does not exist!\n",
            header,
            ptr);
        #endif

        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_KERNEL_PANIC_FAILED
        kernel_panic();
        #endif

        return;
    }

    if (get_nth_block_controll(header->controll_region_start, blockID) != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_START)
    {
        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_IF_FAILED_FREE
        printf("\ncentral_block_memory_allocator_%x: Failed to free block %x, controll word != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_START\n",
            header,
            ptr);
        #endif

        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_KERNEL_PANIC_FAILED
        kernel_panic();
        #endif

        return;
    }

    write_nth_block_controll(header->controll_region_start, CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE, blockID);
    header->number_of_alocated_blocks--;
    header->number_of_free_blocks++;
    blockID++;

    while (get_nth_block_controll(header->controll_region_start, blockID) != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_START &&
        get_nth_block_controll(header->controll_region_start, blockID) != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE)
    {
        write_nth_block_controll(header->controll_region_start, CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE, blockID);
        header->number_of_alocated_blocks--;
        header->number_of_free_blocks++;
        blockID++;
    }
}

void* central_block_memory_allocator_alloc_alligned(size_t size, size_t allignment_as_power_of_two, central_block_memory_allocator_header* header)
{
    size_t allignment_check_offset = 0;
    size_t alligment_check_value = 0;

    #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_ALLOC_FREE
    printf("\ncentral_block_memory_allocator_%x: allocating block <2^%d, %x>\n",
        header, allignment_as_power_of_two, size);
    #endif

    if (allignment_as_power_of_two == 0 || allignment_as_power_of_two <= header->block_size_as_power_of_two)
    {
        // m < n, m ∈ ℤ, n ∈ ℤ
        // aᵐ = a⁽ⁿ ⁺ ⁽ᵐ ⁻ ⁿ⁾⁾, a ∈ ℤ
        // ∴ aᵐ = a⁽ᵐ⁻ⁿ⁾*a^ⁿ
        // This means that if the aligment size is less then the block aligment
        // then all blocks will be alligned
        allignment_as_power_of_two = 0;
    }
    else
    {
        size_t allocation_region_start = (size_t)header->allocation_region_start;
        if (allocation_region_start % (1 << allignment_as_power_of_two) != 0)
        {
            size_t offset = allocation_region_start % (1 << allignment_as_power_of_two);
            allignment_check_offset = (1 << allignment_as_power_of_two) - offset;
        }

        allignment_check_offset >>= header->block_size_as_power_of_two;


        allignment_as_power_of_two -= header->block_size_as_power_of_two; // allignment is now relitive to blocks
        alligment_check_value = (1 << allignment_as_power_of_two) - 1;
    }

    size_t required_blocks = size >> header->block_size_as_power_of_two;
    if (size & ((1 << header->block_size_as_power_of_two) - 1))
        required_blocks += 1;
     
    // This is just devide by 2^block_size_as_power_of_two and round up
    size_t start_block = 0;
    size_t nblocks = 0;

    for (size_t block = allignment_check_offset; block < header->number_of_total_blocks; block++)
    {
        if (allignment_as_power_of_two != 0 && ((block - allignment_check_offset) & alligment_check_value) != 0)
        {
            block -= allignment_check_offset;
            size_t new_block = block >> allignment_as_power_of_two;

            if ((block & ((1 << allignment_as_power_of_two) - 1)) != 0) // Round up
                new_block++;

            new_block <<= allignment_as_power_of_two;
            block = new_block + allignment_check_offset;
        }

        if (get_nth_block_controll(header->controll_region_start, block) != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE)
            continue;

        start_block = block;
        nblocks = 1;
        block++;

        for ( ; block < header->number_of_total_blocks; block++)
        {
            if (nblocks == required_blocks)
            {
                block = header->number_of_total_blocks; // A way to force both for loops to end
                break;
            }
            
            if (get_nth_block_controll(header->controll_region_start, block) != CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_FREE)
                break;
            
            nblocks++;

        }
    }

    if (nblocks != required_blocks)
    {
        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_IF_FAILED_ALLOC
        printf("\ncentral_block_memory_allocator_%x: Failed to allocate block [%x, %d]\n",
            header, size, allignment_as_power_of_two);

        #endif

        #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_KERNEL_PANIC_FAILED
        kernel_panic();
        #endif

        return NULL;
    }

    header->number_of_alocated_blocks += required_blocks;
    header->number_of_free_blocks -= required_blocks;
    for (size_t block = start_block; required_blocks > 0; required_blocks--)
    {
        if (block == start_block)
        {
            write_nth_block_controll(header->controll_region_start, CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_START, block);
        }
        else if (required_blocks > 1)
        {
            write_nth_block_controll(header->controll_region_start, CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_MIDDLE, block);
        }
        else
        {
            write_nth_block_controll(header->controll_region_start, CENTRAL_BLOCK_MEMORY_ALLOCATOR_BLOCK_END, block);
        }
        block++;
    }

    void* return_value = void_ptr_offset_bytes(header->allocation_region_start, start_block << header->block_size_as_power_of_two);
    #ifdef CENTRAL_BLOCK_MEMORY_ALLOCATOR_PRINT_ALLOC_FREE
    printf("returned: %x\n", (size_t)return_value);
    #endif

    return return_value;
}

static uint_fast8_t get_nth_block_controll(uint_fast8_t* controll_region_start, size_t n)
{
    //  ←----------------------- n -----------------------→
    // +============================+======================+  
    // | ←------ byte offset -----→ | ←- bit offset / 2 -→ |
    // | xxxxxxxxxxxxxxxxxxxxxxxxxx |          xx          |
    // +============================+======================+
    //
    // As shown in the diagram above:
    //      byte offset = n >> 2
    //      bit offset = (n & 0b11) * 2

    return (controll_region_start[n >> 2] >> ((n & 0b11) * 2)) & 0b11;
}

static void write_nth_block_controll(uint_fast8_t* controll_region_start, uint_fast8_t data, size_t n)
{
    //  ←----------------------- n -----------------------→
    // +============================+======================+  
    // | ←------ byte offset -----→ | ←- bit offset / 2 -→ |
    // | xxxxxxxxxxxxxxxxxxxxxxxxxx |          xx          |
    // +============================+======================+
    //
    // As shown in the diagram above:
    //      byte offset = n >> 2
    //      bit offset = (n & 0b11) * 2


    uint_fast8_t d = controll_region_start[n >> 2];
    d &= ~(0b11 << ((n & 0b11) * 2));
    d |= data << (n & 0b11) * 2;
    controll_region_start[n >> 2] = d;
}