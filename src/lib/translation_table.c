#include "lib/translation_table.h"

#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/mmu.h"
#include "io/uart.h"

#if PAGE_ALLOCATOR_PAGE_SIZE_AS_POWER_OF_TWO != 21
#error translation_table Are only compatible with a page allocator page size of 2^21
#endif

#pragma region Static function deffinations

// Used to check if a section is definded correctly
// @param section A pointer to the section to check
// @return True if passed, False if failed
static bool s_vailidate_section(translation_table_section_info* section);

// Checks if the given sections overlap
// @param sections A pointer to the sections to check
// @param number_of_sections The number of sections to check
// @return True if passed, False if failed
// @note sections Must be sorted in assinding order relitve to section_start
static bool s_check_sections_for_page_clashing(translation_table_section_info* sections, uint8_t number_of_sections);

// Fills out the PMD for a given section
// @param section A pointer to the section we are using
// @param pmd A pointer to the PMD
// @param only_update_active_buffers_when_ready Used to prevent the function from
// overwriting any buffers untill the new contents are ready, This is achived by allocating new buffers
// @return The number of entrys used
static uint32_t s_fill_out_page_middle_directory(translation_table_section_info* section, translation_table_page_middle_directory_info* pmd, bool only_update_active_buffers_when_ready);

#pragma endregion


bool initialize_translation_table(translation_table_info* table, translation_table_section_info* sections, uint8_t number_of_sections)
{
    uart_puts("Initializing translation table: ");
    uart_put_ptr(table); uart_putc('\n');
    memclr(table, sizeof(translation_table_info));
    table->number_of_sections = number_of_sections;
    table->sections = malloc(sizeof(translation_table_section_info) * number_of_sections);
    memcpy(table->sections, sections, sizeof(translation_table_section_info) * number_of_sections);

    void* last_section_start = 0; // Check the sections are good
    for (int i = 0; i < number_of_sections; i++)
    {
        if (sections[i].section_start <= last_section_start && i !=0)
        {
            uart_puts("Failed to initialize translation table: sections are not sorted!\n"
            "Section: ");
            uart_puti(i); uart_putc('\n');

            return false;
        }

        if (s_vailidate_section(sections + i) == false)
        {
            uart_puts("Failed to initialize translation table: sections: ");
            uart_puti(i); uart_puts(" failed vaildation test\n");

            return false;
        }
    }

    if (!s_check_sections_for_page_clashing(sections, number_of_sections))
        return false;
    
    void* last_virtual_address = void_ptr_offset_bytes(sections[number_of_sections - 1].section_start,
        get_page_allocation_size(sections[number_of_sections - 1].allocation) - 1);
    uint32_t last_pud_index = (uint32_t)((*(size_t*)&last_virtual_address) >>  30);
    table->number_of_page_upper_directory_entrys = last_pud_index + 1;
    table->page_global_directory = aligned_alloc(4096, 8);
    table->page_upper_directory = aligned_alloc(4096, 8 * (last_pud_index + 1));
    table->page_middle_directorys = malloc(sizeof(translation_table_page_middle_directory_info) * number_of_sections);
    memclr(table->page_middle_directorys, sizeof(translation_table_page_middle_directory_info) * number_of_sections);
    memclr(table->page_upper_directory, 8 * (last_pud_index + 1));

    write_page_descriptor(table->page_global_directory,     // Write to PGD
        get_physical_address(table->page_upper_directory),  // Address of PUD first entry
        0x0,                                                // No upper attributes
        0x0,                                                // No lowwer attributes
        true);                                              // Points to page table entry

    for (int i = 0; i < number_of_sections; i++)
    {
        uint32_t section_size = s_fill_out_page_middle_directory(sections + i, table->page_middle_directorys + i, false);
        uint32_t section_pud_size = section_size >> (30 - page_allocator_page_size_as_power_of_two);
        section_pud_size += (section_size & ((1 << (30 - page_allocator_page_size_as_power_of_two)) - 1) ? 1 : 0); // Round Up
        

        void* section_first_virtual_address = sections[i].section_start;
        uint32_t section_first_pud_index = (uint32_t)((*(size_t*)&section_first_virtual_address) >> 30);

        for (uint32_t j = 0; j < section_pud_size; j++)
        {
            write_page_descriptor(
                table->page_upper_directory + section_first_pud_index + j,  // PUD entry
                get_physical_address(table->page_middle_directorys[i].page_middle_directory),        
                                                                            // Address of PMD entry
                0x0,                                                        // No upper attributes
                0x0,                                                        // No lowwer attributes
                true);                                                      // Points to page table entry

        }
    }

    return true;
}

bool remake_translation_table_section(translation_table_info* table, void* section_start, bool only_update_active_buffers_when_ready);

bool append_translation_table_section(translation_table_info* table, translation_table_section_info* section, bool only_update_active_buffers_when_ready);

size_t get_translation_table_memory_mannaged(translation_table_info* table)
{
    size_t size = 0;

    for (int i = 0; i < table->number_of_sections; i++)
        size += get_page_allocation_size(table->sections[i].allocation);

    return size;
}

void destory_translation_table(translation_table_info* table)
{
    uart_puts("Destorying translation table: ");
    uart_put_ptr(table); uart_putc('\n');

    for (int i = 0; i < table->number_of_sections; i++)
        destroy_page_allocation(table->sections[i].allocation);

    free(table->page_global_directory);
    free(table->page_upper_directory);
    free(table->page_middle_directorys);
    free(table->sections);
}

static bool s_vailidate_section(translation_table_section_info* section)
{
    // Check if the section is alligned to 2^30 bytes (2 MiB)
    if ((*(size_t*)&section->section_start) & ((1 << 30) - 1) != 0)
        return false;

    return true;
}

static bool s_check_sections_for_page_clashing(translation_table_section_info* sections, uint8_t number_of_sections)
{
    // Assuming sections is a sorted array we just need to do this
    // if sections[i].section_start + size(sections[i]) <= sections[i + 1].section_start
    // for i = {0, 1, 2, 3, ..., number_of_sections - 3, number_of_sections -2 }

    for (int i = 0; i < number_of_sections - 1; i++)
    {
        if ((*(size_t*)&(sections + i)->section_start) + get_page_allocation_size(sections[i].allocation) > (*(size_t*)&(sections + i + 1)->section_start))
        {
            uart_puts("page clash between: ");
            uart_puti(i); uart_puts(" and ");
            uart_puti(i + 1); uart_puts(".\n");
            return false;
        }
            
    }

    return true;
}

// Note: could have an issuse where the active pmd is freed,
// then a interupt trys to allocate memory, and overwrites the pmd,
// before the current pgd / pud is updated...
// There for: updating kernel translation sections needs to be done with interupts off
static uint32_t s_fill_out_page_middle_directory(translation_table_section_info* section, translation_table_page_middle_directory_info* pmd, bool only_update_active_buffers_when_ready)
{
    size_t section_size = get_page_allocation_size(section->allocation);
    
    uint32_t required_entrys = section_size >> 21 + ((section_size & ((1 << 21) - 1)) ? 1 : 0); 
    
    uint64_t* pmd_buffer = NULL;

    if (pmd->number_of_page_middle_directory_entrys != required_entrys 
        || only_update_active_buffers_when_ready == true 
        || pmd->page_middle_directory == NULL)
    {
        pmd->number_of_page_middle_directory_entrys = required_entrys;
        pmd_buffer = aligned_alloc(4096, required_entrys * 8);
        memclr(pmd_buffer, required_entrys * 8);

        if (only_update_active_buffers_when_ready == false)
        {
            if (pmd->page_middle_directory != NULL)
                free(pmd->page_middle_directory);
            pmd->page_middle_directory = pmd_buffer;
        }
    }
    else
        pmd_buffer = pmd->page_middle_directory;

    page_allocation_info* allocation = section->allocation;
    
    int pmd_buffer_index = 0 ;

    while (allocation != NULL)
    {
        for (uint32_t i = 0; i < allocation->size; i++, pmd_buffer_index++)
        {
            write_page_descriptor(pmd_buffer + pmd_buffer_index,    // PMD entry pointer
                (void*)(((size_t)(allocation->first_page) + i) 
                << page_allocator_page_size_as_power_of_two),       // Page pointer
                section->upper_attributes,                          // Upper attibutes
                section->lowwer_attributes,                         // Lowwer attibutes)
                false);                                             // We're not pointer to a pte here
        }

        allocation = allocation->next;
    }
    


    if (only_update_active_buffers_when_ready == true)
    {
        free(pmd->page_middle_directory);
        pmd->page_middle_directory = pmd_buffer;
    }

    return required_entrys;
}