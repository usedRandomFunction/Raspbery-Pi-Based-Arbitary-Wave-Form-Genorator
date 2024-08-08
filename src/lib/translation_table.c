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

// Fills out / allocates the PUD for a given  table also updates PGD
// @param table The table to update the PUD for
// @param only_update_active_buffers_when_ready Used to prevent the function from
// overwriting any buffers untill the new contents are ready, This is achived by allocating new buffers
// @return True if passed, False if failed
// @note The PMDs must be filled out before calling this function. 
// When updating PGD only_update_active_buffers_when_ready is ignored
static bool s_fill_out_page_upper_directory(translation_table_info* table, bool only_update_active_buffers_when_ready);

// Invalides all caches around the given translation talbe
// @param table The table to work with
static void s_invalid_caching_around_translation_table(translation_table_info* table);
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
    

    table->page_global_directory = aligned_alloc(4096, 4096);
    table->page_middle_directorys = malloc(sizeof(translation_table_page_middle_directory_info) * number_of_sections);
    memclr(table->page_middle_directorys, sizeof(translation_table_page_middle_directory_info) * number_of_sections);
    memclr(table->page_global_directory, 4096);


    for (int i = 0; i < number_of_sections; i++)
    {
        s_fill_out_page_middle_directory(sections + i, table->page_middle_directorys + i, false);
    }

    if (!s_fill_out_page_upper_directory(table, false))
    {
        uart_puts("Failed to initialize translation table: failed to fill PUD\n");
        return false;
    }

    s_invalid_caching_around_translation_table(table);

    return true;
}

void print_translation_table(translation_table_info* table)
{
    uart_puts("Translation table: ");
    uart_put_ptr(table); uart_puts(" debug.\n");

    uint32_t last_pud_index = table->number_of_page_upper_directory_entrys - 1;

    uint32_t number_of_page_upper_directories = (last_pud_index >> 8) + ((last_pud_index & ((1 << 8) - 1)) ? 1 : 0);

    uart_puts("page global directory:\n");
    for (int i = 0; i < number_of_page_upper_directories; i++)
    {
        print_page_descriptor(table->page_global_directory + i);
    }

    uart_puts("\npage upper directory:\n");

    for (int i = 0; i < table->number_of_page_upper_directory_entrys; i++)
    {
        print_page_descriptor(table->page_upper_directory + i);
    }

    uart_puts("\nsections: ");

    for (int i = 0; i < table->number_of_sections; i++)
    {
        uart_puts("section: ");
        uart_puti(i);
        uart_puts("\npage middle directory:\n");
        
        translation_table_page_middle_directory_info* section = table->page_middle_directorys + i;

        for (int i = 0; i < section->number_of_page_middle_directory_entrys; i++)
        {
            print_page_descriptor(section->page_middle_directory + i);
        }

        uart_putc('\n');
    }
}

bool remake_translation_table_section(translation_table_info* table, int section_id, bool only_update_active_buffers_when_ready)
{
    uart_puts("Translation table: ");
    uart_put_ptr(table);
    uart_puts("\nRemaking section: ");
    uart_puti(section_id); uart_putc('\n');

    if (table->number_of_sections <= section_id)
    {
        uart_puts("Failed to remake section, section does not exist!\n");
        return false;
    }

    translation_table_page_middle_directory_info* pmd = &table->page_middle_directorys[section_id];
    translation_table_section_info* section = &table->sections[section_id];

    size_t section_size = get_page_allocation_size(section->allocation);
    size_t last_section_size_GiB = (pmd->number_of_page_middle_directory_entrys >> 9) + ((pmd->number_of_page_middle_directory_entrys & ((1 << 9) - 1)) ? 1 : 0);
    size_t section_size_GiB = (section_size >> 30) + ((section_size & ((1 << 30) - 1)) ? 1 : 0);

    if (section_size_GiB > last_section_size_GiB && table->number_of_sections > (section_id + 1)) // If we're not the last section, make sure we dont overwrite stuff
    {
        size_t next_section_first_address = *(size_t*)table->sections[section_id + 1].section_start;
        size_t section_last_address = *(size_t*)section->section_start + section_size;

        if (section_last_address >= next_section_first_address)
        {
            uart_puts("Failed to remake section, end address classes with section: ");
            uart_puti(section_id + 1); uart_puts("!\n");
            return false;
        }
    }

    s_fill_out_page_middle_directory(section, pmd, only_update_active_buffers_when_ready);

    if (!s_fill_out_page_upper_directory(table, only_update_active_buffers_when_ready))
    {
        uart_puts("Failed to remake section: failed to fill PUD\n");
        return false;
    }

    uart_puts("success!\n");
    s_invalid_caching_around_translation_table(table);
    invalidate_tlb();

    return true;
}

bool insert_translation_table_section(translation_table_info* table, translation_table_section_info* section, bool only_update_active_buffers_when_ready)
{
    int target_section_id = table->number_of_sections;

    uart_puts("Translation table: ");
    uart_put_ptr(table);
    uart_puts("\nInserting section: [start]: ");
    uart_put_ptr(section->section_start); 
    uart_puts(", [allocation]: ");
    uart_put_ptr(section->allocation); 
    uart_putc('\n');

    for (int i = 0; i < table->number_of_sections; i++) // Check if we are inserting not apending
    {
        if (table->sections[i].section_start < section->section_start)
            continue; // Wait untill the new section is bellow the current section

        size_t section_size = get_page_allocation_size(section->allocation);

        size_t next_section_start = *((size_t*)&table->sections[i].section_start);
        size_t section_end = *((size_t*)&section->section_start) + section_size;

        if (next_section_start < section_end)
        {
            uart_puts("Failed to insert section, virtual address clashes with section: ");
            uart_puti(i); uart_putc('\n');
            return false;
        }

        target_section_id = i;
        break;
    }
    // Now we know where its going, we can check if it can fit there
    size_t last_section_size = get_page_allocation_size(table->sections[target_section_id - 1].allocation);
    size_t last_section_end = *((size_t*)&table->sections[target_section_id - 1].section_start) + last_section_size;

    if (last_section_end > *((size_t*)&section->section_start))
    {
        uart_puts("Failed to insert section, virtual address clashes with section: ");
        uart_puti(target_section_id - 1); uart_putc('\n');
        return false;
    }
    // Now it fits, lets set up the buffers

    translation_table_page_middle_directory_info* page_middle_directorys = malloc(sizeof(translation_table_section_info) * table->number_of_sections + 1);
    memclr(page_middle_directorys, sizeof(translation_table_page_middle_directory_info) * (table->number_of_sections + 1));

    memcpy(page_middle_directorys, table->page_middle_directorys, sizeof(translation_table_page_middle_directory_info) * target_section_id);
    memcpy(page_middle_directorys + target_section_id + 1, table->page_middle_directorys + target_section_id, 
        sizeof(translation_table_page_middle_directory_info) * (table->number_of_sections - target_section_id));
    free(table->page_middle_directorys);
    table->page_middle_directorys = page_middle_directorys;

    translation_table_section_info* sections = malloc(sizeof(translation_table_section_info) * (table->number_of_sections + 1));
    memclr(sections, sizeof(translation_table_section_info) * table->number_of_sections + 1);

    memcpy(sections, table->sections, sizeof(translation_table_section_info) * target_section_id);
    memcpy(sections + target_section_id + 1, table->sections + target_section_id, 
        sizeof(translation_table_section_info) * (table->number_of_sections - target_section_id));
    free(table->sections);
    table->sections = sections;
    // Buffers have been updated to include space for the new section

    memcpy(sections + target_section_id, section, sizeof(translation_table_section_info));
    section = NULL; // We dont want to use this one anymore as it will not update the values in the table object

    table->number_of_sections++;

    s_fill_out_page_middle_directory(&table->sections[target_section_id], 
        &table->page_middle_directorys[target_section_id], false);

    s_fill_out_page_upper_directory(table, only_update_active_buffers_when_ready);

    uart_puts("Success!\n");
    s_invalid_caching_around_translation_table(table);
    invalidate_tlb();
    
    return true;
}

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
    // Check if the section is alligned to 2^30 bytes (1 GiB)
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
    size_t last_section_size_GiB = (pmd->number_of_page_middle_directory_entrys >> 9) + ((pmd->number_of_page_middle_directory_entrys & ((1 << 9) - 1)) ? 1 : 0);
    size_t section_size_GiB = (section_size >> 30) + ((section_size & ((1 << 30) - 1)) ? 1 : 0);
    
    uint32_t required_entrys = (section_size >> 21) + ((section_size & ((1 << 21) - 1)) ? 1 : 0); 
    
    uint64_t* pmd_buffer = NULL;

    if (pmd->page_middle_directory == NULL || only_update_active_buffers_when_ready == true || last_section_size_GiB != section_size_GiB)
    {
        pmd_buffer = aligned_alloc(4096, 4096 * section_size_GiB);

        if (only_update_active_buffers_when_ready == false)
        {
            if (pmd->page_middle_directory != NULL)
                free(pmd->page_middle_directory);
            pmd->page_middle_directory = pmd_buffer;
        }
    }
    else
        pmd_buffer = pmd->page_middle_directory;
    memclr(pmd_buffer, 4096 * section_size_GiB);

    pmd->number_of_page_middle_directory_entrys = required_entrys;

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

static bool s_fill_out_page_upper_directory(translation_table_info* table, bool only_update_active_buffers_when_ready)
{
    uint64_t* pud_buffer = NULL;

    void* last_virtual_address = void_ptr_offset_bytes(table->sections[table->number_of_sections - 1].section_start,
        get_page_allocation_size(table->sections[table->number_of_sections - 1].allocation) - 1);
    uint32_t last_pud_index = (uint32_t)((*(size_t*)&last_virtual_address) >>  30);
    uint32_t previous_number_of_page_upper_directories = table->number_of_page_upper_directory_entrys - 1;
    previous_number_of_page_upper_directories = (previous_number_of_page_upper_directories >> 8) + ((previous_number_of_page_upper_directories & ((1 << 8) - 1)) ? 1 : 0);
    uint32_t number_of_page_upper_directories = (last_pud_index >> 8) + ((last_pud_index & ((1 << 8) - 1)) ? 1 : 0);
    table->number_of_page_upper_directory_entrys = last_pud_index + 1;
    
    if (number_of_page_upper_directories > (1 << 8))
    {
        uart_puts("Failed to fill out page upper directory: out of address space\n");
        return false;
    }

    if (table->page_upper_directory == NULL || only_update_active_buffers_when_ready == true || number_of_page_upper_directories != previous_number_of_page_upper_directories)
    {
        pud_buffer = aligned_alloc(4096, 4096 * number_of_page_upper_directories);

        // memclr(pud_buffer, 4096 * number_of_page_upper_directories);
        if (only_update_active_buffers_when_ready == false)
        {
            if (table->page_upper_directory != NULL)
                free(table->page_upper_directory);
            table->page_upper_directory = pud_buffer;
        }
    }
    else
        pud_buffer = table->page_upper_directory;
    memclr(pud_buffer, 4096 * number_of_page_upper_directories);// Should be here but the program just halts for no reason


    
    for (int i = 0; i < table->number_of_sections; i++)
    {
        uint32_t section_size = table->page_middle_directorys[i].number_of_page_middle_directory_entrys;
        uint32_t section_pud_size = section_size >> (30 - page_allocator_page_size_as_power_of_two);
        section_pud_size += (section_size & ((1 << (30 - page_allocator_page_size_as_power_of_two)) - 1) ? 1 : 0); // Round Up
        

        void* section_first_virtual_address = table->sections[i].section_start;
        uint32_t section_first_pud_index = (uint32_t)((*(size_t*)&section_first_virtual_address) >> 30);

        for (uint32_t j = 0; j < section_pud_size; j++)
        {
            write_page_descriptor(
                pud_buffer + section_first_pud_index + j,  // PUD entry
                get_physical_address(table->page_middle_directorys[i].page_middle_directory),        
                                                                            // Address of PMD entry
                0x0,                                                        // No upper attributes
                0x0,                                                        // No lowwer attributes
                true);                                                      // Points to page table entry
        }
    }


    if (only_update_active_buffers_when_ready == true)
    {
        free(table->page_upper_directory);
        table->page_upper_directory = pud_buffer;
    }

    for (int i = 0; i < number_of_page_upper_directories; i++)
    {
        write_page_descriptor(table->page_global_directory + i,             // Write to PGD
            get_physical_address(pud_buffer + 4096 * i),   // Address of PUD first entry
            0x0,                                                            // No upper attributes
            0x0,                                                            // No lowwer attributes
            true);                                                          // Points to page table entry
    }

    memclr(table->page_global_directory + number_of_page_upper_directories, 4096 - (number_of_page_upper_directories * 8));

    return true;
}

static void s_invalid_caching_around_translation_table(translation_table_info* table)
{
    invalidate_data_cache_of_size(table->page_global_directory, 4096);
    invalidate_data_cache_of_size(table->page_upper_directory, table->number_of_page_upper_directory_entrys * 4096);

    for (int i = 0; i < table->number_of_sections; i++)
    {
        translation_table_page_middle_directory_info* pmd = &table->page_middle_directorys[i];
        size_t section_size_GiB = (pmd->number_of_page_middle_directory_entrys >> 9) + 
        ((pmd->number_of_page_middle_directory_entrys & ((1 << 9) - 1)) ? 1 : 0);

        invalidate_data_cache_of_size(pmd->page_middle_directory, section_size_GiB * 4096);
    }
}