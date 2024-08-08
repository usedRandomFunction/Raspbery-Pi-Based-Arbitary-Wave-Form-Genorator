#include "io/framebuffer.h"

#include "lib/translation_table.h"
#include "lib/page_allocator.h"
#include "io/propertyTags.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/mmu.h"
#include "io/uart.h"

extern translation_table_info kernel_translation_table;

static uint32_t s_framebuffer_bytes_per_line;
static uint8_t* s_framebuffer_pointer;
static uint32_t s_framebuffer_height;
static uint32_t s_framebuffer_width;
static bool s_framebuffer_is_rgb; // True if RGB false if BGR

bool initialize_framebuffer(uint32_t target_width, uint32_t target_height)
{
    s_framebuffer_bytes_per_line = 0;
    s_framebuffer_pointer = (NULL);
    s_framebuffer_is_rgb = false;
    s_framebuffer_height = 0;
    s_framebuffer_width = 0;

    uart_puts("Initializing frame buffer!\n");

    property_tag_set_physical_display_width_height set_physical_display_width_height; 
    set_physical_display_width_height.header.buffersize = PROPERTY_TAG_SET_PHYSICAL_DISPLAY_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_physical_display_width_height.header.tagID = PROPERTY_TAG_ID_SET_PHYS_WIDTH_HEIGHT;
    set_physical_display_width_height.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_physical_display_width_height.height = 600;
    set_physical_display_width_height.width = 1024;

    property_tag_set_physical_display_width_height_responce* set_physical_display_width_height_responce = 
        (property_tag_set_physical_display_width_height_responce*)get_property_tag((property_tag*)&set_physical_display_width_height, 
        aligned_alloc_noncachable, free_noncachable_memory);

    if (set_physical_display_width_height_responce == NULL)
    {
        uart_puts("Failed to initialize frame buffer: set physical display width height failed!\n");
        return false;
    }

    s_framebuffer_height = set_physical_display_width_height_responce->height;
    s_framebuffer_width = set_physical_display_width_height_responce->width;

    free_noncachable_memory(set_physical_display_width_height_responce);

    property_tag_set_virtual_offset set_virtual_offset;
    set_virtual_offset.header.buffersize = PROPERTY_TAG_SET_VIRTUAL_OFFSET_REQUEST_RESPONSE_SIZE;
    set_virtual_offset.header.tagID = PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET;
    set_virtual_offset.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_offset.X = 0;
    set_virtual_offset.Y = 0;

    property_tag* throw_away_tag;

    throw_away_tag = get_property_tag((property_tag*)&set_virtual_offset, aligned_alloc_noncachable, free_noncachable_memory);
    if (throw_away_tag == NULL)
    {
        uart_puts("Failed to initialize frame buffer: set virtual offset failed!\n");
        return false;
    }

    free_noncachable_memory(throw_away_tag);

    property_tag_set_virtual_buffer_width_height set_virtual_buffer_width_height;
    set_virtual_buffer_width_height.header.buffersize = PROPERTY_TAG_SET_VIRTUAL_BUFFER_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_virtual_buffer_width_height.header.tagID = PROPERTY_TAG_ID_SET_VIRT_WIDTH_HEIGHT;
    set_virtual_buffer_width_height.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_buffer_width_height.height = 7;
    set_virtual_buffer_width_height.width = s_framebuffer_width;

    throw_away_tag = get_property_tag((property_tag*)&set_virtual_buffer_width_height, aligned_alloc_noncachable, free_noncachable_memory);
    if (throw_away_tag == NULL)
    {
        uart_puts("Failed to initialize frame buffer: set virtual buffer width height failed!\n");
        return false;
    }

    free_noncachable_memory(throw_away_tag);

    property_tag_set_depth set_depth;
    set_depth.header.buffersize = PROPERTY_TAG_SET_DEPTH_REQUEST_RESPONSE_SIZE;
    set_depth.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_depth.header.tagID = PROPERTY_TAG_ID_SET_DEPTH;
    set_depth.bits_per_pixel = 32;

    throw_away_tag = get_property_tag((property_tag*)&set_depth, aligned_alloc_noncachable, free_noncachable_memory);
    if (throw_away_tag == NULL)
    {
        uart_puts("Failed to initialize frame buffer: set depth failed!\n");
        return false;
    }

    free_noncachable_memory(throw_away_tag);

    property_tag_set_pixel_order set_pixel_order;
    set_pixel_order.header.buffersize = PROPERTY_TAG_SET_PIXEL_ORDER_REQUEST_RESPONSE_SIZE;
    set_pixel_order.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_pixel_order.header.tagID = PROPERTY_TAG_ID_SET_PIXEL_ORDER;
    set_pixel_order.state = PROPERTY_TAG_PIXEL_ORDER_RGB;

    property_tag_set_pixel_order_responce* set_pixel_order_responce = (property_tag_set_pixel_order_responce*)
        get_property_tag((property_tag*)&set_pixel_order, aligned_alloc_noncachable, free_noncachable_memory);
    if (set_pixel_order_responce == NULL)
    {
        uart_puts("Failed to initialize frame buffer: set pixel order failed!\n");
        return false;
    }

    s_framebuffer_is_rgb = set_pixel_order_responce->state == PROPERTY_TAG_PIXEL_ORDER_RGB;

    free_noncachable_memory(set_pixel_order_responce);

    property_tag_allocate_buffer allocate_buffer;
    allocate_buffer.header.buffersize = PROPERTY_TAG_ALLOCATE_BUFFER_REQUEST_RESPONSE_SIZE;
    allocate_buffer.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    allocate_buffer.header.tagID = PROPERTY_TAG_ID_ALLOCATE_BUFFER;
    allocate_buffer.alignment_in_bytes = 4096;

    property_tag_allocate_buffer_responce* framebuffer = (property_tag_allocate_buffer_responce* )get_property_tag((property_tag*)&allocate_buffer, aligned_alloc_noncachable, free_noncachable_memory);
    if (framebuffer == NULL)
    {
        uart_puts("Failed to initialize frame buffer: failed to allocate framebuffer!\n");
        return false;
    }

    property_tag_get_pitch get_pitch;
    get_pitch.header.buffersize = PROPERTY_TAG_GET_PITCH_REQUEST_RESPONSE_SIZE;
    get_pitch.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_pitch.header.tagID = PROPERTY_TAG_ID_GET_PITCH;

    property_tag_get_pitch_responce* pitch_repsonce = (property_tag_get_pitch_responce*)get_property_tag((property_tag*)&get_pitch, aligned_alloc_noncachable, free_noncachable_memory);

    if (pitch_repsonce == NULL)
    {
        uart_puts("Failed to initialize frame buffer: failed to get bytes per line!\n");
        free_noncachable_memory(framebuffer);
        return false;
    }

    s_framebuffer_bytes_per_line = pitch_repsonce->bytes_per_line;

    ptrdiff_t framebuffer_offset = 0;

    page_allocation_info* framebuffer_allocation = create_new_page_allocation_for_unmanaged_continuous_physical_address(
        VC_address_to_arm(*(void**)&framebuffer->base_address), framebuffer->size, &framebuffer_offset);

    translation_table_section_info table_section;
    table_section.allocation = framebuffer_allocation;
    table_section.lowwer_attributes = MMU_LOWER_ATTRIBUTES_NON_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_section.upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_section.section_start = FRAMEBUFFER_VIRUTAL_ADDRESS_BASE; // We dont include the FFFF prefix here
    insert_translation_table_section(&kernel_translation_table, &table_section, true);
    free_noncachable_memory(framebuffer);

    s_framebuffer_pointer = void_ptr_offset_bytes(FRAMEBUFFER_VIRUTAL_ADDRESS_BASE, framebuffer_offset + KERNEL_MEMORY_PREFIX);

    
    uart_puts("Successfully initialized frame buffer of size: ");
    uart_putui(s_framebuffer_width);
    uart_puts(" x ");
    uart_putui(s_framebuffer_height);
    uart_putc('\n');

    return true;
}

void set_framebuffer_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= s_framebuffer_width || y >= s_framebuffer_height)
    {
        return;
    }

    uint32_t offset = x * 4 + y * s_framebuffer_bytes_per_line;

    if (s_framebuffer_is_rgb)
    {
        s_framebuffer_pointer[offset] = r;
        s_framebuffer_pointer[offset + 2] = b;
    }
    else
    {
        s_framebuffer_pointer[offset] = b;
        s_framebuffer_pointer[offset + 2] = r;
    }
    s_framebuffer_pointer[offset + 1] = g;
}

uint32_t get_framebuffer_height()
{
    return s_framebuffer_height;
}

uint32_t get_framebuffer_width()
{
    return s_framebuffer_width;
}