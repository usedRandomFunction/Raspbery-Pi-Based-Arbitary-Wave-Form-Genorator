#include "lib/translation_table.h"
#include "lib/page_allocator.h"
#include "io/propertyTags.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/mmu.h"
#include "io/uart.h"

extern translation_table_info kernel_translation_table;

void frame_buffer_test()
{
    // This is a dirty test and doesn't works as the address of the allocation isn't mapped, 
    // this still uploaded to back up the changes as i perpare to update the tables
    property_tag_set_physical_display_width_height set_physical_display_width_height; 
    set_physical_display_width_height.header.buffersize = PROPERTY_TAG_SET_PHYSICAL_DISPLAY_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_physical_display_width_height.header.tagID = PROPERTY_TAG_ID_SET_PHYS_WIDTH_HEIGHT;
    set_physical_display_width_height.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_physical_display_width_height.height = 600;
    set_physical_display_width_height.width = 1024;

    write_property_tag((property_tag*)&set_physical_display_width_height, aligned_alloc_noncachable, free_noncachable_memory);

    property_tag_set_virtual_offset set_virtual_offset;
    set_virtual_offset.header.buffersize = PROPERTY_TAG_SET_VIRTUAL_OFFSET_REQUEST_RESPONSE_SIZE;
    set_virtual_offset.header.tagID = PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET;
    set_virtual_offset.header.request = PROPERTY_TAG_PROCESS_REQUEST;

    write_property_tag((property_tag*)&set_virtual_offset, aligned_alloc_noncachable, free_noncachable_memory);


    property_tag_set_virtual_buffer_width_height set_virtual_buffer_width_height;
    set_virtual_buffer_width_height.header.buffersize = PROPERTY_TAG_SET_VIRTUAL_BUFFER_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_virtual_buffer_width_height.header.tagID = PROPERTY_TAG_ID_SET_VIRT_WIDTH_HEIGHT;
    set_virtual_buffer_width_height.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_buffer_width_height.height = 600;
    set_virtual_buffer_width_height.width = 1024;

    write_property_tag((property_tag*)&set_virtual_buffer_width_height, aligned_alloc_noncachable, free_noncachable_memory);

    property_tag_set_depth set_depth;
    set_depth.header.buffersize = PROPERTY_TAG_SET_DEPTH_REQUEST_RESPONSE_SIZE;
    set_depth.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_depth.header.tagID = PROPERTY_TAG_ID_SET_DEPTH;
    set_depth.bits_per_pixel = 32;

    write_property_tag((property_tag*)&set_depth, aligned_alloc_noncachable, free_noncachable_memory);

    property_tag_set_pixel_order set_pixel_order;
    set_pixel_order.header.buffersize = PROPERTY_TAG_SET_PIXEL_ORDER_REQUEST_RESPONSE_SIZE;
    set_pixel_order.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_pixel_order.header.tagID = PROPERTY_TAG_ID_SET_PIXEL_ORDER;

    write_property_tag((property_tag*)&set_pixel_order, aligned_alloc_noncachable, free_noncachable_memory);

    property_tag_allocate_buffer allocate_buffer;
    allocate_buffer.header.buffersize = PROPERTY_TAG_ALLOCATE_BUFFER_REQUEST_RESPONSE_SIZE;
    allocate_buffer.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    allocate_buffer.header.tagID = PROPERTY_TAG_ID_ALLOCATE_BUFFER;
    allocate_buffer.alignment_in_bytes = 4096;

    property_tag_allocate_buffer_responce* framebuffer = (property_tag_allocate_buffer_responce* )get_property_tag((property_tag*)&allocate_buffer, aligned_alloc_noncachable, free_noncachable_memory);

    // property_tag_get_pitch get_pitch;
    // get_pitch.header.buffersize = PROPERTY_TAG_GET_PITCH_REQUEST_RESPONSE_SIZE;
    // get_pitch.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    // get_pitch.header.tagID = PROPERTY_TAG_ID_GET_PITCH;

    // property_tag_get_pitch_responce* pitch_repsonce = (property_tag_get_pitch_responce*)get_property_tag((property_tag*)&get_pitch, aligned_alloc_noncachable, free_noncachable_memory)l
    
    uint32_t page = framebuffer->base_address & 0x3FFFFFFF;
    page /= page_allocator_page_size_bytes;
    uint32_t diff = framebuffer->base_address % page_allocator_page_size_bytes;
    

    page_allocation_info* framebuffer_allocation = malloc(sizeof(page_allocation_info));
    framebuffer_allocation->first_page = page;
    framebuffer_allocation->size = (framebuffer->size + diff) / page_allocator_page_size_bytes + ((framebuffer->size + diff) % page_allocator_page_size_bytes == 0 ? 0 : 1); 
    framebuffer_allocation->next = NULL;

    translation_table_section_info table_section;
    table_section.allocation = framebuffer_allocation;
    table_section.lowwer_attributes = MMU_LOWER_ATTRIBUTES_nGnRnE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_section.upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_section.section_start = (void*)0x000100000000; // We dont include the FFFF prefix here
    insert_translation_table_section(&kernel_translation_table, &table_section, true);

    uart_puts("attempt to draw now!\n");

    uint32_t* pixel_buffer =  (uint32_t*)(size_t)(0xFFFF000100000000 + diff);

    for (int i = 0; i < framebuffer->size / 4; i++)
    {
        pixel_buffer[i] = i;
    }


    // free_noncachable_memory(pitch_repsonce);
    free_noncachable_memory(framebuffer);

}