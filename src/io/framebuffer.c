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
    s_framebuffer_pointer = NULL;
    s_framebuffer_is_rgb = false;
    s_framebuffer_height = FRAMEBUFFER_HEIGHT;
    s_framebuffer_width = FRAMEBUFFER_WIDTH;

    uart_puts("Initializing frame buffer!\n");

    const uint32_t tag_buffer_size_bytes = sizeof(property_tag_set_physical_display_width_height) +
        sizeof(property_tag_set_virtual_buffer_width_height) +
        sizeof(property_tag_set_virtual_offset) +
        sizeof(property_tag_set_depth) +
        sizeof(property_tag_set_pixel_order) +
        sizeof(property_tag_allocate_buffer) +
        sizeof(property_tag_get_pitch);

    uint8_t property_tags_to_send[tag_buffer_size_bytes]; // just set up the buffer
    ptrdiff_t buffer_offset = 0;

    // ================ Note =============== //
    // If you change the order in this table //
    // you noeed to update the return buffer //
    //              table aswell             //
    // ================ Note =============== //

    property_tag_set_physical_display_width_height* set_physical_display_width_height = 
        (property_tag_set_physical_display_width_height*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_physical_display_width_height);

    property_tag_set_virtual_buffer_width_height* set_virtual_buffer_width_height = 
        (property_tag_set_virtual_buffer_width_height*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_buffer_width_height);

    property_tag_set_virtual_offset* set_virtual_offset = 
        (property_tag_set_virtual_offset*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_offset);

    property_tag_set_depth* set_depth = 
        (property_tag_set_depth*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_depth);

    property_tag_set_pixel_order* set_pixel_order = 
        (property_tag_set_pixel_order*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_pixel_order);

    property_tag_allocate_buffer* allocate_buffer = 
        (property_tag_allocate_buffer*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_allocate_buffer);

    property_tag_get_pitch* get_pitch = 
        (property_tag_get_pitch*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_get_pitch);

    // ================ Note =============== //
    // If you change the order in this table //
    // you noeed to update the return buffer //
    //              table aswell             //
    // ================ Note =============== //

    set_physical_display_width_height->header.buffersize = PROPERTY_TAG_SET_PHYSICAL_DISPLAY_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_physical_display_width_height->header.tagID = PROPERTY_TAG_ID_SET_PHYS_WIDTH_HEIGHT;
    set_physical_display_width_height->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_physical_display_width_height->height = s_framebuffer_height;
    set_physical_display_width_height->width = s_framebuffer_width;

    set_virtual_buffer_width_height->header.buffersize = PROPERTY_TAG_SET_VIRTUAL_BUFFER_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_virtual_buffer_width_height->header.tagID = PROPERTY_TAG_ID_SET_VIRT_WIDTH_HEIGHT;
    set_virtual_buffer_width_height->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_buffer_width_height->height = s_framebuffer_height;
    set_virtual_buffer_width_height->width = s_framebuffer_width;

    set_virtual_offset->header.buffersize = PROPERTY_TAG_SET_VIRTUAL_OFFSET_REQUEST_RESPONSE_SIZE;
    set_virtual_offset->header.tagID = PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET;
    set_virtual_offset->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_offset->X = 0;
    set_virtual_offset->Y = 0;

    set_depth->header.buffersize = PROPERTY_TAG_SET_DEPTH_REQUEST_RESPONSE_SIZE;
    set_depth->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_depth->header.tagID = PROPERTY_TAG_ID_SET_DEPTH;
    set_depth->bits_per_pixel = 32;

    set_pixel_order->header.buffersize = PROPERTY_TAG_SET_PIXEL_ORDER_REQUEST_RESPONSE_SIZE;
    set_pixel_order->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_pixel_order->header.tagID = PROPERTY_TAG_ID_SET_PIXEL_ORDER;
    set_pixel_order->state = PROPERTY_TAG_PIXEL_ORDER_RGB;

    allocate_buffer->header.buffersize = PROPERTY_TAG_ALLOCATE_BUFFER_REQUEST_RESPONSE_SIZE;
    allocate_buffer->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    allocate_buffer->header.tagID = PROPERTY_TAG_ID_ALLOCATE_BUFFER;
    allocate_buffer->alignment_in_bytes = 4096;

    get_pitch->header.buffersize = PROPERTY_TAG_GET_PITCH_REQUEST_RESPONSE_SIZE;
    get_pitch->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_pitch->header.tagID = PROPERTY_TAG_ID_GET_PITCH;

    uint8_t* property_tag_return = (uint8_t*)get_property_tags((property_tag*)property_tags_to_send, tag_buffer_size_bytes, aligned_alloc, free);

    if (property_tag_return == NULL)
    {
        uart_puts("Failed to initialize frame buffer: get_property_tags() failed!\n");
        return false;
    }


    buffer_offset = 0;

    
    // ================ Note =============== //
    // If you change the order in this table //
    //    you noeed to update the request    //
    //          buffer table aswell          //
    // ================ Note =============== //

    property_tag_set_physical_display_width_height_responce* set_physical_display_width_height_responce = 
        (property_tag_set_physical_display_width_height_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_physical_display_width_height_responce);

    property_tag_set_virtual_buffer_width_height_responce* set_virtual_buffer_width_height_responce = 
        (property_tag_set_virtual_buffer_width_height_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_buffer_width_height_responce);

    property_tag_set_virtual_offset_responce* set_virtual_offset_responce = 
        (property_tag_set_virtual_offset_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_offset_responce);

    property_tag_set_depth_responce* set_depth_responceresponce = 
        (property_tag_set_depth_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_depth_responce);

    property_tag_set_pixel_order_responce* set_pixel_order_responce = 
        (property_tag_set_pixel_order_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_pixel_order_responce);

    property_tag_allocate_buffer_responce* allocate_buffer_responce = 
        (property_tag_allocate_buffer_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_allocate_buffer_responce);

    property_tag_get_pitch_responce* get_pitch_responce = 
        (property_tag_get_pitch_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_get_pitch_responce);

    // ================ Note =============== //
    // If you change the order in this table //
    //    you noeed to update the request    //
    //          buffer table aswell          //
    // ================ Note =============== //



    s_framebuffer_is_rgb = set_pixel_order_responce->state == PROPERTY_TAG_PIXEL_ORDER_RGB;
    s_framebuffer_height = set_physical_display_width_height_responce->height;
    s_framebuffer_width = set_physical_display_width_height_responce->width;
    s_framebuffer_bytes_per_line = get_pitch_responce->bytes_per_line;

    ptrdiff_t framebuffer_offset = 0;

    page_allocation_info* framebuffer_allocation = create_new_page_allocation_for_unmanaged_continuous_physical_address(
        VC_address_to_arm(*(void**)&allocate_buffer_responce->base_address), allocate_buffer_responce->size, &framebuffer_offset);
    free(property_tag_return);

    translation_table_section_info table_section;
    table_section.allocation = framebuffer_allocation;
    table_section.lowwer_attributes = MMU_LOWER_ATTRIBUTES_NON_CACHABLE | MMU_LOWER_ATTRIBUTES_ACCESS_BIT;
    table_section.upper_attributes = MMU_UPPER_ATTRIBUTES_EXECUTE_NEVER;
    table_section.section_start = FRAMEBUFFER_VIRUTAL_ADDRESS_BASE; // We dont include the FFFF prefix here
    insert_translation_table_section(&kernel_translation_table, &table_section, true);

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