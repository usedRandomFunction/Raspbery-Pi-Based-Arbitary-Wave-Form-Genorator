#include "app/data_output.h"
#include "io/propertyTags.h"
#include "app/startup.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/alloc.h"
#include "lib/math.h"
#include "io/gpio.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{


    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", 1.2f);
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", 1.3f);
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_SDRAM, "SDRam", 1.2f);

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

    uint32_t* pixel_buffer =  (uint32_t*)(size_t)framebuffer->base_address;

    for (int i = 0; i < framebuffer->size; i++)
    {
        pixel_buffer[i] = i;
    }


    // free_noncachable_memory(pitch_repsonce);
    free_noncachable_memory(framebuffer);

    return 0;
}

void* operator new(size_t size)
{
    void * p = malloc(size);
    return p;
}

void operator delete(void * p)
{
    free(p);
}
