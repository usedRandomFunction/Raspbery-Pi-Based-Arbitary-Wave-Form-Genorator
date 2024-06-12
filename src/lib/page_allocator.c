#include "lib/page_allocator.h"

#include "io/propertyTags.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "io/uart.h"

#pragma region static_functions

// gets the availability of a page
// @param page The page to check
// @return True if allocated, False if free
static bool s_get_page_availability(uint32_t page);

// Sets a page to the given a availability
// @param page The page to set
// @param is_allocated True if allocated
static void s_set_page_availability(uint32_t page, bool is_allocated);

// Incresses the size of a page allocaiton to match the new size requirments
// @param allocation A pointer to the allocation to messure
// @param new_size The new target size (in bytes) for the allocation
// @param current_size The current size (in bytes) of the allocaiton
// @return True if success False if failer
static bool s_grow_page_allocation(page_allocation_info* allocation, size_t new_size, size_t current_size);

// Decreases the size of a page allocaiton to match the new size requirments
// @param allocation A pointer to the allocation to messure
// @param new_size The new target size (in bytes) for the allocation
// @param current_size The current size (in bytes) of the allocaiton
// @return True if success False if failer
static bool s_shrink_page_allocation(page_allocation_info* allocation, size_t new_size, size_t current_size);
#pragma endregion

static uint32_t s_number_of_page_allocations;
static uint32_t s_total_number_of_pages;
static uint32_t s_total_allocated_pages;

// A array with one bit per page, were 0 is free and 1 is allocated
static uint8_t* s_page_availability; 

const size_t page_allocator_page_size_as_power_of_two = PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO;
const size_t page_allocator_page_size_bytes = 1 << page_allocator_page_size_as_power_of_two;

bool initialize_page_allocator()
{
    s_number_of_page_allocations = 0;
    s_total_allocated_pages = 0;

    property_tag_get_arm_memory memory_tag;
    memory_tag.buffersize = PROPERTY_TAG_GET_ARM_MEMORY_REQUEST_RESPONSE_SIZE;
    memory_tag.request = PROPERTY_TAG_PROCESS_REQUEST;
    memory_tag.tagID = PROPERTY_TAG_ID_GET_ARM_MEMORY;

    property_tag_get_arm_memory_responce* memory_tag_responce = (property_tag_get_arm_memory_responce*)get_property_tag(&memory_tag, aligned_alloc, free);

    if (memory_tag_responce == false)
    {
        uart_puts("Failed to initialize page allocator: failed to get arm memory size!\n");
        return false;
    }

    size_t arm_memory_size = memory_tag_responce->size;
    free(memory_tag_responce);
    s_total_number_of_pages = arm_memory_size >> page_allocator_page_size_as_power_of_two;

    size_t required_bytes = s_total_number_of_pages >> 3 + (s_total_number_of_pages & 0x7 ? 1 : 0); // Devide by 8 round up
    s_page_availability = malloc(required_bytes);

    if (s_page_availability == NULL)
    {
        uart_puts("Failed to initialize page allocator: failed to allocate page availability table!\n");
        return false;
    }

    memclr(s_page_availability, required_bytes);

    uart_puts("initialized page allocator with ");
    uart_putui(s_total_number_of_pages);
    uart_puts(" pages (");
    uart_putui((s_total_number_of_pages << page_allocator_page_size_as_power_of_two) / 1024 / 1024);
    uart_puts(" Mib)\n");

    return true;
}

page_allocation_info* create_new_page_allocation(size_t size)
{
    uint32_t required_pages = (uint32_t)(size >> page_allocator_page_size_as_power_of_two);
    required_pages += (size & (page_allocator_page_size_bytes - 1) ? 1 : 0); // Devide round up
    page_allocation_info* root =  malloc(sizeof(page_allocation_info));
    if (root == NULL)
    {
        uart_puts("Failed to create page alloaction: malloc failed!\n");
        return NULL;
    }

    page_allocation_info* current_alloc = root;
    uint32_t pages_left = required_pages;
    uint32_t page_index = 0;
    bool reached_end_of_free = true;
    memclr(root, sizeof(page_allocation_info));

    while (pages_left > 0 && page_index < s_total_number_of_pages)
    {
        if (s_get_page_availability(page_index++) == true)
        {
            if (reached_end_of_free == true)
                continue;
            reached_end_of_free = true;

            page_allocation_info* new_alloc =  malloc(sizeof(page_allocation_info));
            if (new_alloc == NULL)
            {
                uart_puts("Failed to create page alloaction: malloc failed!\n");
                destroy_page_allocation(root);
                return NULL;
            }
            memclr(new_alloc, sizeof(page_allocation_info));
            current_alloc->next = new_alloc;
            current_alloc = new_alloc;

            continue;
        }

        if (reached_end_of_free == true) // First free block
        {
            current_alloc->first_page = page_index - 1;
        }

        pages_left--;
        reached_end_of_free = false;
        s_set_page_availability(page_index - 1, true);
        if (current_alloc->size++ == 0) // Is first entry, also incoment
        {
            current_alloc->first_page = page_index - 1;
        }
    }

    if (pages_left != 0)
    {
        uart_puts("Failed to create page alloaction: out of memory!\n");
        destroy_page_allocation(root);
        return NULL;
    }


    return root;
}

page_allocation_info* create_new_page_allocation_at_continuous_physical_address(void* physical_address_start, size_t size)
{
    size_t physical_address = *(size_t*)&physical_address_start;
    size_t page_start = physical_address >> page_allocator_page_size_as_power_of_two;
    size += physical_address % page_allocator_page_size_bytes;
    size_t required_pages = size >> page_allocator_page_size_as_power_of_two;
    required_pages += (size & (page_allocator_page_size_bytes - 1) ? 1 : 0); // Devide round up

    bool area_is_empty = true;

    for (int i = 0; i < required_pages && i < (s_total_number_of_pages - page_start); i++)
    {
        if (s_get_page_availability(page_start + i) == true)
        {
            area_is_empty = false;
            break;
        }
    }

    if (area_is_empty == false)
    {
        uart_puts("Failed to create page alloaction: region is in use!\n");
        return NULL;
    }

    for (int i = 0; i < required_pages; i++)
        s_set_page_availability(page_start + i, true);

    page_allocation_info* allocation = malloc(sizeof(page_allocation_info));
    
    if (allocation == NULL)
    {
        uart_puts("Failed to create page alloaction: malloc failed!\n");
        return NULL;
    }

    s_total_allocated_pages += required_pages;
    allocation->first_page = page_start;
    allocation->size = required_pages;
    allocation->next = NULL;

    uart_puts("Created page allocation ");
    uart_put_ptr(allocation);
    uart_puts(" with size ");
    uart_putui((required_pages << page_allocator_page_size_as_power_of_two) / 1024);
    uart_puts(" kib\n");

    return allocation;
}

size_t get_page_allocation_size(page_allocation_info* allocation)
{
    uint32_t size = 0;

    while (allocation != NULL)
    {
        size += allocation->size;
        allocation = allocation->next;
    }

    return size << page_allocator_page_size_as_power_of_two;
}

bool resize_page_allocation(page_allocation_info* allocation, size_t new_size)
{
    size_t current_size = get_page_allocation_size(allocation);
    
    if (current_size == new_size)
        return true;

    if (current_size < new_size)
        return s_grow_page_allocation(allocation, new_size, current_size);

    return s_shrink_page_allocation(allocation, new_size, current_size);
}

void destroy_page_allocation(page_allocation_info* allocation)
{
    uart_puts("Destorying page: allocation ");
    uart_put_ptr(allocation);
    uart_putc('\n');

    while (allocation != NULL)
    {
        page_allocation_info* next = allocation->next;
        
        for (int i = 0; i < allocation->size; i++)
        {
            s_set_page_availability(allocation->first_page + i, false);
        }

        free(allocation);
        allocation = next;
    }
}

uint32_t get_total_allocated_pages()
{
    return s_total_allocated_pages;
}

uint32_t get_total_free_pages()
{
    return s_total_number_of_pages - s_total_allocated_pages;
}

static bool s_get_page_availability(uint32_t page)
{
    uint32_t index = page / 8;
    int offset = page & 0x7; // same as page % 8

    return  (s_page_availability[index] & (1 << offset)) >> offset;
}

static void s_set_page_availability(uint32_t page, bool is_allocated)
{
    uint32_t index = page / 8;
    int offset = page & 0x7; // same as page % 8

    uint8_t mask = ~(1 << offset);
    s_page_availability[index] &= mask;
    s_page_availability[index] |= 1 << offset;
}

static bool s_grow_page_allocation(page_allocation_info* allocation, size_t new_size, size_t current_size)
{
    uint32_t size_dif = new_size - current_size;
    uart_puts("Expanding page allocation: ");
    uart_put_ptr(allocation);
    uart_puts(" by ");
    uart_putui((size_dif) / 1024);
    uart_puts(" kib (");
    uart_putui((new_size) / 1024);
    uart_puts(" kib total)\nCreating extention allocation\n");

    page_allocation_info* extention_allocation = create_new_page_allocation(size_dif);

    if (extention_allocation == NULL)
    {
        uart_puts("Failed to expand page allocation: ");
        uart_put_ptr(allocation); uart_putc('\n');

        return false;
    }
    // We could end it here but i will handle the case where to two allocations touch at the end and start respectivly
    page_allocation_info* last_allocation_section = allocation;

    while (last_allocation_section->next != NULL)
    {
        last_allocation_section = last_allocation_section->next;
    }

    if (last_allocation_section->first_page + last_allocation_section->size != extention_allocation->first_page)
    {   // It is not that edge case never mind
        last_allocation_section->next = extention_allocation;

        uart_puts("Success!\n");
        return true;
    }
    // It *is* that edge case, so we merge them
    last_allocation_section->size += extention_allocation->size;
    last_allocation_section->next = extention_allocation->next;
    free(extention_allocation);

    return true;
}

static bool s_shrink_page_allocation(page_allocation_info* allocation, size_t new_size, size_t current_size)
{
    size_t size_dif = current_size - new_size;
    uint32_t required_pages = (uint32_t)(new_size >> page_allocator_page_size_as_power_of_two);
    required_pages += (new_size & (page_allocator_page_size_bytes - 1) ? 1 : 0); // Devide round up
    uart_puts("Shinking page allocation: ");
    uart_put_ptr(allocation);
    uart_puts(" by ");
    uart_putui((size_dif) / 1024);
    uart_puts(" kib (to ");
    uart_putui((new_size) / 1024);
    uart_puts(" kib)\n");

    page_allocation_info* to_shink = allocation;

    uint32_t current_pages = 0;

    while (allocation != NULL)
    {
        current_pages += allocation->size;
        if (current_pages >= required_pages)
            break;

        allocation = allocation->next;
    }
    uint32_t pages_to_free = current_pages - required_pages;

    if (current_pages < required_pages || pages_to_free == 0)
    {
        uart_puts("Failed to shink allocation, allocation is smaller to expected!\n");
        return false;
    }

    if (to_shink->next != NULL)
    {
        uart_puts("Deleting allocation section after cut off.");
        destroy_page_allocation(to_shink);
    }

    uint32_t first_page_to_free = to_shink->first_page + to_shink->size - pages_to_free;

    for (uint32_t i = 0; i < pages_to_free; i++)
    {
        s_set_page_availability(first_page_to_free + i, true);
    }

    uart_puts("Success!\n");
    return true;
}