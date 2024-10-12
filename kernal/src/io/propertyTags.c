#include "io/propertyTags.h"
#include "io/mailbox.h"
#include "lib/memory.h"
#include "io/printf.h"
#include "lib/mmu.h"


// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

struct property_tag_buffer_header
{
    uint32_t size;
    uint32_t request_response_code; 
};

typedef struct property_tag_buffer_header property_tag_buffer_header;

static void* s_prepare_property_tag_buffer(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);

void write_property_tags(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    property_tag_buffer_header* ptr = s_prepare_property_tag_buffer(buffer, buffer_size, _malloc, _free);
    size_t physical_address = (size_t)get_physical_address(ptr);
    
    uint32_t returnValue = mailbox_write_read_physcial_alliged_address((void*)physical_address, 8);

    if ((uint32_t)physical_address != returnValue || ptr->request_response_code != PROPERTY_TAG_REQUEST_SUCCESSFUL)
    {  
        printf("\nFailed to write proptery tag: ");
        if ((uint32_t)physical_address != returnValue)
        {
            printf("address != returnValue\n return address: %x\n", returnValue);
        }
        else
        {
            printf("reponce code: %x\n", ptr->request_response_code);
        }

        return;
    }

    _free(ptr);
}

property_tag* get_property_tags(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    property_tag_buffer_header* ptr = s_prepare_property_tag_buffer(buffer, buffer_size, _malloc, _free);
    size_t physical_address = (size_t)get_physical_address(ptr);
    
    uint32_t returnValue = mailbox_write_read_physcial_alliged_address((void*)physical_address, 8);

    if ((uint32_t)physical_address != returnValue || ptr->request_response_code != PROPERTY_TAG_REQUEST_SUCCESSFUL)
    {  
        printf("\nFailed to write proptery tag: ");
        if ((uint32_t)physical_address != returnValue)
        {
            printf("address != returnValue\n return address: %x\n", returnValue);
        }
        else
        {
            printf("reponce code: %x\n", ptr->request_response_code);
        }

        return NULL;
    }

    size_t return_size = ptr->size - sizeof(property_tag_buffer_header);
    property_tag* retrun_buffer = _malloc(0, return_size);
    memcpy(retrun_buffer, ptr + 1, return_size);
    _free(ptr);
    return retrun_buffer;
}


static void* s_prepare_property_tag_buffer(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    size_t true_buffer_size = buffer_size + sizeof(property_tag_buffer_header);
    // size_t buffer_end_tag_offset = true_buffer_size;
    true_buffer_size += sizeof(uint32_t); // For end tag

    if (true_buffer_size & (4 - 1)) // Is not multiple of 4
    {
        true_buffer_size >>= 2;
        true_buffer_size++;
        true_buffer_size <<= 2; // Round up
    }
    property_tag_buffer_header* ptr = _malloc(16, true_buffer_size); // The address must be alliged to 16 and some
    memset(ptr, true_buffer_size, 0);
    ptr->request_response_code = 0x00000000;
    ptr->size = true_buffer_size;
    memcpy(ptr + 1, buffer, buffer_size);

    invalidate_data_cache_of_size(ptr, true_buffer_size);

    return ptr;
}
