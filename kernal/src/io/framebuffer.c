#include "io/framebuffer.h"

#include "run_time_kernal_config.h"
#include "lib/translation_table.h"
#include "lib/page_allocator.h"
#include "io/propertyTags.h"
#include "lib/memory.h"
#include "io/putchar.h"
#include "io/printf.h"
#include "lib/math.h"
#include "lib/mmu.h"

// !======== Note about naming !========!
// While outside of this file frame buffers
// are abstract objects given as a ID to write
// functions, that apear on the display;
// in this file they are called virtual frame
// buffers. This is becouse the videocard
// only has one frame buffer and we are just using
// offsets
// !======== Note about naming !========!

bool s_is_frambuffer_initialized = false;

extern translation_table_info kernel_translation_table;

static uint32_t s_framebuffer_bytes_per_line;
static uint8_t* s_framebuffer_pointer;
static uint32_t s_framebuffer_height;
static uint32_t s_framebuffer_size;
static uint32_t s_framebuffer_width;
static int s_number_of_framebuffers;
static int s_active_framebuffer;
static bool s_framebuffer_is_rgb; // True if RGB false if BGR

// Uses the property tags to set up the hardware frame buffer
// @param physical_width Display width
// @param physical_height Display height
// @param virtual_width Width of the frame buffer its self
// @param virtual_height Height of the frame buffer its self
// @param virtual_offset_x Offset of the physical display inside the virtual buffer
// @param virtual_offset_y Offset of the physical display inside the virtual buffer
// @param overscan_left Overscan controll
// @param overscan_right Overscan controll
// @param overscan_top Overscan controll
// @param overscan_bottom Overscan controll
// @return Base address of the frame buffer, or NULL if failed
static void* s_create_framebuffer(uint32_t physical_width, uint32_t physical_height, uint32_t virtual_width, uint32_t virtual_height,
    uint32_t virtual_offset_x, uint32_t virtual_offset_y, uint32_t overscan_left, uint32_t overscan_right,
    uint32_t overscan_top, uint32_t overscan_bottom);

// Creatse or moves the physical address of the virtual memory mapping used by the frame buffer
// @param base_address Pointer to start of frame buffer (Physcial address)
// @return True if success false if failed
static bool s_create_or_move_framebuffer_vmem_mapping(void* base_address);

bool initialize_framebuffer()
{
    s_is_frambuffer_initialized = false;
    uint32_t buffer_height = display_height * (minimum_number_of_frame_buffers + (is_running_in_qemu ? 1 : 0));
    uint32_t buffer_width = display_width;

    printf("Initializing frame buffer!\n");

    void* base_address = s_create_framebuffer(
        display_width, display_height,    // Physical display size
        buffer_width, buffer_height,    // Virtual buffer size
        0, 0, 0, 0, 0, 0);              // Offsets and overscan

    if (base_address == NULL)
        return false;

    if (s_create_or_move_framebuffer_vmem_mapping(base_address) == false)
        return false;

    // Blank the screen
    framebuffer_fill_rect(0, 0, s_framebuffer_width, s_framebuffer_height, FRAMEBUFFER_RGB(0, 0, 0));
    putchar_on_frame_buffer_blanked();
    
    s_is_frambuffer_initialized = true;
    s_number_of_framebuffers = minimum_number_of_frame_buffers;
    s_active_framebuffer = 0;
    printf("Successfully initialized frame buffer\n");


    return true;
}

int active_framebuffer(int buffer) // UNDONE handle qemu work around
{
    if (buffer == -1)
        return s_active_framebuffer;                            // Handle explicit case if buffer -1

    if (buffer >= s_number_of_framebuffers || buffer < 0)       // Handle case of buffer being out of rnage
        return s_active_framebuffer;

    if (buffer == s_active_framebuffer)                         // Handle case of switching to active buffer
        return s_active_framebuffer;

    int last_active_frame_buffer = s_active_framebuffer;        // This has to be done otherwise printf will 
    s_active_framebuffer = -1;                                  // Write on top of the frame buffer, if switching from 0 to anything

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    buffer += is_running_in_qemu ? 1 : 0;

    if (is_running_in_qemu)     // Since the offset function isn't working in QEMU, we will use screen copys to fake it
    {                           // and a few other ticks in other functions (Issue #24)
        printf("Copying buffer (%d + 1), to buffer 0 (workaround for issue #24)\n", buffer - 1);

        framebuffer_screen_copy(0, display_height * buffer,     // Copy new buffer
            get_display_width(), get_display_height(), 0, 0);   // Paste it to buffer 0
    }

    property_tag_set_virtual_offset offset_tag;
    offset_tag.header.buffersize = PROPERTY_TAG_SET_VIRTUAL_OFFSET_REQUEST_RESPONSE_SIZE;
    offset_tag.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    offset_tag.header.tagID = PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET;
    offset_tag.X = 0;
    offset_tag.Y = display_height * buffer;


    property_tag_set_virtual_offset_responce* responce = 
        (property_tag_set_virtual_offset_responce*)get_property_tag((property_tag*)&offset_tag, aligned_alloc, free);
    
    if (responce == NULL || responce->X != offset_tag.X || responce->Y != offset_tag.Y)
    {
        printf("Failed to set virtual buffer offset.\n");
        free(responce);

        s_active_framebuffer = last_active_frame_buffer;
        return s_active_framebuffer;
    }

    free(responce);

    printf("Set framebuffer active buffer to %d (0, %d)\n", buffer, display_height * buffer);

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    buffer -= is_running_in_qemu ? 1 : 0;

    s_active_framebuffer = buffer;
    return s_active_framebuffer;
}

int request_frame_buffers(int nbuffers)
{
     if (nbuffers == -1)
        return s_number_of_framebuffers;

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    nbuffers += is_running_in_qemu ? 1 : 0;
    nbuffers = clamp(nbuffers, maximum_number_of_frame_buffers, minimum_number_of_frame_buffers);

    if (nbuffers == s_number_of_framebuffers)
        return s_number_of_framebuffers;

    if (nbuffers < s_number_of_framebuffers && (!allways_shirnk_frame_buffer_if_possible))
    {
        s_number_of_framebuffers = nbuffers;
        return s_number_of_framebuffers;
    }

    int new_active_framebuffer_id = s_active_framebuffer >= nbuffers ? 0 : s_active_framebuffer;
    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    new_active_framebuffer_id += is_running_in_qemu ? 1 : 0;

    s_is_frambuffer_initialized = false;
    uint32_t buffer_height = display_height * nbuffers;
    uint32_t buffer_width = display_width;

    printf("Creating video card frame buffer!\n");

    void* base_address = s_create_framebuffer(
        display_width, display_height,                  // Physical display size
        buffer_width, buffer_height,                    // Virtual buffer size
        0, display_height * new_active_framebuffer_id,  // Offsets
        0, 0, 0, 0);                                    // Overscan

    if (base_address == NULL)
        return s_number_of_framebuffers;

    if (s_create_or_move_framebuffer_vmem_mapping(base_address) == false)
        return s_number_of_framebuffers;

    // Blank the screen
    framebuffer_fill_rect(0, 0, s_framebuffer_width, s_framebuffer_height, FRAMEBUFFER_RGB(0, 0, 0));
    putchar_on_frame_buffer_blanked();
    
    s_is_frambuffer_initialized = true;
    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    s_number_of_framebuffers = nbuffers - (is_running_in_qemu ? 1 : 0);
    s_active_framebuffer = new_active_framebuffer_id - (is_running_in_qemu ? 1 : 0);

    printf("Success!\n");
    return s_number_of_framebuffers;
}

void set_framebuffer_pixel(uint32_t x, uint32_t y, display_color color)
{
    if (x >= s_framebuffer_width || y >= s_framebuffer_height)
    {
        return;
    }


    if (!s_framebuffer_is_rgb)
    {
        uint32_t red  = color & 0xFF;
        uint32_t blue = (color >> 16) & 0xFF; 
        color = (color & 0xFF00FF00) | blue | (red << 16); 
    }

    size_t offset = x * 4 + y * s_framebuffer_bytes_per_line;
    
    uint32_t* pixel = (uint32_t*)(s_framebuffer_pointer + offset);
    *pixel = color;
}

void set_display_pixel(uint32_t x, uint32_t y, display_color color, int buffer)
{
    if (buffer < 0 || buffer >= s_number_of_framebuffers)
        return;

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    buffer += is_running_in_qemu ? 1 : 0;

    set_framebuffer_pixel(x, y + display_height * buffer, color);

    if (buffer == (s_active_framebuffer + 1) && is_running_in_qemu) // Write to buffer zero as well, if selected buffer is active, issue #24
    {
        set_framebuffer_pixel(x, y, color);
    }
}

void framebuffer_screen_copy(uint32_t copy_area_start_x, uint32_t copy_area_start_y, uint32_t copy_area_size_x, uint32_t copy_area_size_y, uint32_t paste_area_start_x, uint32_t paste_area_start_y)
{
    for (int y_offset = 0; y_offset < copy_area_size_y; y_offset++)
    {
        uint32_t copy_offset_start = (copy_area_start_x) * 4 + 
            (copy_area_start_y + y_offset) * s_framebuffer_bytes_per_line;
        
        uint32_t paste_offset_start = (paste_area_start_x) * 4 + 
            (paste_area_start_y + y_offset) * s_framebuffer_bytes_per_line;

        memcpy(s_framebuffer_pointer + paste_offset_start, s_framebuffer_pointer + copy_offset_start, copy_area_size_x * 4);
    }
}

void display_screen_copy(uint32_t copy_area_start_x, uint32_t copy_area_start_y, uint32_t copy_area_size_x, uint32_t copy_area_size_y, uint32_t paste_area_start_x, uint32_t paste_area_start_y, int buffer)
{
    if (buffer < 0 || buffer >= s_number_of_framebuffers)
        return;

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    buffer += is_running_in_qemu ? 1 : 0;
    uint32_t buffer_offset = display_height * buffer;

    framebuffer_screen_copy(copy_area_start_x, copy_area_start_y + buffer_offset,
        copy_area_size_x, copy_area_size_y,
        paste_area_start_x, paste_area_start_y + buffer_offset);

    if (buffer == (s_active_framebuffer + 1) && is_running_in_qemu)   // Copy to buffer zero as well if selected buffer is active, issue #24
    {
        framebuffer_screen_copy(copy_area_start_x, copy_area_start_y + buffer_offset,
        copy_area_size_x, copy_area_size_y,
        paste_area_start_x, paste_area_start_y);
    }
}

void framebuffer_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, display_color color)
{
    for (int y = y0 ; y <= y1; y++)
    {
        for (int x = x0 ; x <= x1; x++)
        {
            set_framebuffer_pixel(x, y, color);
        }
    }
}

void display_fill_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, display_color color, int buffer)
{
    if (buffer < 0 || buffer >= s_number_of_framebuffers)
        return;

    // Buffer 0 is used to fix issue #24, but it is not exposed in the API
    buffer += is_running_in_qemu ? 1 : 0;
    uint32_t buffer_offset = display_height * buffer;

    framebuffer_fill_rect(x0, y0 + buffer_offset, x1, y1 + buffer_offset, color);

    if (buffer == (s_active_framebuffer + 1) && is_running_in_qemu) // Write to buffer zero as well, if selected buffer is active, issue #24
    {
        framebuffer_fill_rect(x0, y0, x1, y1, color);
    }


}

void set_display_overscan(uint32_t top, uint32_t bottom, uint32_t left, uint32_t right)
{
    property_tag_set_overscan overscan_tag;
    overscan_tag.header.buffersize = PROPERTY_TAG_SET_OVERSCAN_REQUEST_RESPONSE_SIZE;
    overscan_tag.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    overscan_tag.header.tagID = PROPERTY_TAG_ID_SET_OVERSCAN;
    overscan_tag.top = top;
    overscan_tag.bottom = bottom;
    overscan_tag.left = left;
    overscan_tag.right = right;

    printf("Setting overscan to %d, %d, %d, %d (top, bottom, left, right)\n", top, bottom, left, right);

    property_tag_set_overscan_responce* responce = 
        (property_tag_set_overscan_responce*)get_property_tag((property_tag*)&overscan_tag, aligned_alloc, free);
    
    
    if (responce == NULL || responce->top != top || responce->bottom != bottom || responce->left != left || responce->right != right )
    {
        printf("Failed to set overscan.\n");
        free(responce);

        return;
    }

    free(responce);
}

uint32_t get_display_height()
{
    return display_height;
}

uint32_t get_display_width()
{
    return display_width;
}

bool is_frambuffer_initialized()
{
    return s_is_frambuffer_initialized;
}

void framebuffer_on_user_app_exit()
{
    active_framebuffer(0);
    
    if (allways_shirnk_frame_buffer_if_possible)
        request_frame_buffers(minimum_number_of_frame_buffers);

    framebuffer_fill_rect(0, display_height, display_width - 1, s_framebuffer_height - 1, FRAMEBUFFER_RGB(0, 0, 0));
}

void* s_create_framebuffer(uint32_t physical_width, uint32_t physical_height, uint32_t virtual_width, uint32_t virtual_height,
    uint32_t virtual_offset_x, uint32_t virtual_offset_y, uint32_t overscan_left, uint32_t overscan_right,
    uint32_t overscan_top, uint32_t overscan_bottom)
{
    printf("Creating new frame buffer.\n"
        "Physical size: %d by %d px,\n"
        "Virtual size: %d by %d px\n"
        "Virtual offset: (%d, %d) px\n"
        "Overscan: %d top, %d bottom, %d left, %d right\n", 
        physical_width, physical_height,
        virtual_width, virtual_height,
        virtual_offset_x, virtual_offset_y,
        overscan_top, overscan_bottom, overscan_left, overscan_right);

    const uint32_t tag_buffer_size_bytes = sizeof(property_tag_set_physical_display_width_height) +
        sizeof(property_tag_set_virtual_buffer_width_height) +
        sizeof(property_tag_set_virtual_offset) +
        sizeof(property_tag_set_overscan) +
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

    property_tag_set_overscan* set_overscan =
        (property_tag_set_overscan*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_overscan);

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
    set_physical_display_width_height->height = physical_height;
    set_physical_display_width_height->width = physical_width;

    set_virtual_buffer_width_height->header.buffersize = PROPERTY_TAG_SET_VIRTUAL_BUFFER_WIDTH_HEIGHT_REQUEST_RESPONSE_SIZE;
    set_virtual_buffer_width_height->header.tagID = PROPERTY_TAG_ID_SET_VIRT_WIDTH_HEIGHT;
    set_virtual_buffer_width_height->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_buffer_width_height->height = virtual_height;
    set_virtual_buffer_width_height->width = virtual_width;

    set_virtual_offset->header.buffersize = PROPERTY_TAG_SET_VIRTUAL_OFFSET_REQUEST_RESPONSE_SIZE;
    set_virtual_offset->header.tagID = PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET;
    set_virtual_offset->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_virtual_offset->X = virtual_offset_x;
    set_virtual_offset->Y = virtual_offset_y;

    set_overscan->header.buffersize = PROPERTY_TAG_SET_OVERSCAN_REQUEST_RESPONSE_SIZE;
    set_overscan->header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_overscan->header.tagID = PROPERTY_TAG_ID_SET_OVERSCAN;
    set_overscan->bottom = overscan_bottom;
    set_overscan->right = overscan_right;
    set_overscan->left = overscan_left;
    set_overscan->top = overscan_top;

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
        printf("Failed to initialize frame buffer: get_property_tags() failed!\n");
        return NULL;
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

    // Commented out to make gcc happy (unused variable)
    // property_tag_set_virtual_buffer_width_height_responce* set_virtual_buffer_width_height_responce = 
    //  (property_tag_set_virtual_buffer_width_height_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_buffer_width_height_responce);

    // Commented out to make gcc happy (unused variable)
    // property_tag_set_virtual_offset_responce* set_virtual_offset_responce = 
    //  (property_tag_set_virtual_offset_responce*) (property_tag_return + buffer_offset);
    buffer_offset += sizeof(property_tag_set_virtual_offset_responce);

    // property_tag_set_overscan_responce* set_overscan_responce =
        // (property_tag_set_overscan_responce*) (property_tags_to_send + buffer_offset);
    buffer_offset += sizeof(property_tag_set_overscan_responce);

    // Commented out to make gcc happy (unused variable)
    // property_tag_set_depth_responce* set_depth_responceresponce = 
    //  (property_tag_set_depth_responce*) (property_tag_return + buffer_offset);
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
    s_framebuffer_height = set_virtual_buffer_width_height->height;
    s_framebuffer_width = set_virtual_buffer_width_height->width;
    display_height = set_physical_display_width_height_responce->height;
    display_width = set_physical_display_width_height_responce->width;
    s_framebuffer_bytes_per_line = get_pitch_responce->bytes_per_line;
    s_framebuffer_size = allocate_buffer_responce->size;

    void* base_address = (void*)(size_t)allocate_buffer_responce->base_address;

    free(property_tag_return);

    return base_address;
}

bool s_create_or_move_framebuffer_vmem_mapping(void* base_address)
{   
    translation_table_section_info table_section;
    translation_table_section_info* target_section = &table_section;
    table_section.attributes = MMU_ATTRIBUTES_NON_CACHABLE | MMU_ATTRIBUTES_ACCESS_BIT | MMU_ATTRIBUTES_EXECUTE_NEVER;
    table_section.section_start = FRAMEBUFFER_VIRUTAL_ADDRESS_BASE; // We dont include the FFFF prefix here
    int target_section_id = -1;

    for (int i = 0; i < kernel_translation_table.number_of_sections; i++)               // First we check if the section allready exists
    {
        translation_table_section_info* section = kernel_translation_table.sections + i;

        if (section->section_start < FRAMEBUFFER_VIRUTAL_ADDRESS_BASE)         // Skip if its not the target section
            continue; 
        else if (section->section_start > FRAMEBUFFER_VIRUTAL_ADDRESS_BASE)
            break;                                                              // No need to keep going after we're above it

        printf("Remaking frame buffer vem memory mapping\n");
        destroy_page_allocation(section->allocation);

        target_section_id = i;
        break;
    }
    
    ptrdiff_t framebuffer_offset = 0;
    page_allocation_info* framebuffer_allocation = create_new_page_allocation_for_unmanaged_continuous_physical_address(
        VC_address_to_arm(base_address), s_framebuffer_size, &framebuffer_offset);

    target_section->allocation = framebuffer_allocation;

    if (target_section_id >= 0)
    {
        if (remake_translation_table_section(&kernel_translation_table, target_section_id, true) == false)
        {
            printf("Failed to %s vmem mappings for frame buffer\n", "remake");
            return false;
        }
    }
    else
    {
        if (insert_translation_table_section(&kernel_translation_table, target_section, true)== false)
        {
            printf("Failed to %s vmem mappings for frame buffer\n", "create");
            return false;
        }
    }

    s_framebuffer_pointer = void_ptr_offset_bytes(FRAMEBUFFER_VIRUTAL_ADDRESS_BASE, framebuffer_offset + KERNEL_MEMORY_PREFIX);

    printf("Success!\n");

    return true;
}