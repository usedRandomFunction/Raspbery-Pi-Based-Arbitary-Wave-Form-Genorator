#include "gui/elements.h"
#include "gui/drawing_functions.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"



void draw_element_recursive(gui_element* element, gui_vec2 offset, int target_buffer)
{
    if (element->draw)
        element->draw(element, offset, target_buffer);

    gui_element** buffer= (gui_element**)element->sub_elements.ptr;

    offset.x += element->position.x;
    offset.y += element->position.y;

    for (int i = 0; i < element->sub_elements.number_of_entrys; i++)
    {
        gui_element* sub_element = buffer[i];

        if (sub_element->flags & GUI_ELEMENT_FLAGS_HIDDEN)
            continue;               // Skip hidden elements

        draw_element_recursive(sub_element, offset, target_buffer);
    }
}

gui_element_colors* get_element_colors(gui_element* element)
{
    if (element->flags & GUI_ELEMENT_FLAGS_DISABLED)
        return &element->color_disabled;

    if (element->flags & GUI_ELEMENT_FLAGS_FOCUSED)
        return &element->color_focused;

    return &element->color;
}

size_t find_element_index(gui_element* element, dynamic_array* buffer)
{
    // Since the buffer its not sorted in anyway we're just going to brute force it

    gui_element** buffer_ptr = (gui_element**)element->sub_elements.ptr;

    for (size_t i = 0; i < element->sub_elements.number_of_entrys; i++)
    {
        if (buffer_ptr[i] == element)
            return i;
    }

    return -1;
}

void free_element_recursive(gui_element* element)
{
    free_sub_elements_recursive(element);

    delete_dynamic_array(&element->sub_elements);

    if (element->data)
        free(element->data);
}

void free_sub_elements_recursive(gui_element* element)
{
    gui_element** buffer= (gui_element**)element->sub_elements.ptr;

    for (int i = 0; i < element->sub_elements.number_of_entrys; i++)
    {
        gui_element* sub_element = buffer[i];

        free_element_recursive(sub_element);
    }
}

void remove_and_free_element_recursive(gui_element* element, dynamic_array* buffer)
{
    size_t index = find_element_index(element, buffer);

    if (index != -1)
        remove_dynamic_array_entry(index, buffer);

    free_element_recursive(element);
}

void initialize_standered_element_values(gui_element* element)
{
    memclr(element, sizeof(gui_element));

    element->border_width_focused = 1;
    
    initialize_dynamic_array(sizeof(gui_element*), 0, &element->sub_elements);

    element->color.background = FRAMEBUFFER_RGB(1, 100, 192); // #0164c0
    element->color.forground = FRAMEBUFFER_RGB(235, 235, 235); // #ebebeb
    element->color.border = FRAMEBUFFER_RGB(235, 235, 235); // #ebebeb

    element->color_focused.background = FRAMEBUFFER_RGB(1, 124, 190); // #017cbe
    element->color_focused.forground = FRAMEBUFFER_RGB(255, 255, 255); // #FFFFFF
    element->color_focused.border = FRAMEBUFFER_RGB(255, 255, 255); // #FFFFFF

    element->color_disabled.background = FRAMEBUFFER_RGB(0, 62, 120); // #003e78
    element->color_disabled.forground = FRAMEBUFFER_RGB(150, 150, 150); // #969696
    element->color_disabled.border = FRAMEBUFFER_RGB(150, 150, 150); // #969696
}

gui_element* create_element(dynamic_array* buffer)
{
    gui_element* element = malloc(sizeof(gui_element));
    
    if (!element)
    {
        printf("[Error] Failed to create element, unable to allocate memory!\n");
        return NULL;
    }

    memclr(element, sizeof(gui_element));

    initialize_dynamic_array(sizeof(gui_element*), 0, &element->sub_elements);
    insert_dynamic_array(&element, buffer->number_of_entrys, buffer);

    return element;
}

gui_element* possition_element_vertically(gui_element* element, int padding, gui_element* last_element)
{
    if (element == NULL || last_element == NULL)
        return element;

    element->position.x = last_element->position.x;

    if (padding < 0)
    {
        padding++;

        element->position.y = last_element->position.y - element->size.y + padding;
    }
    else
    {
        element->position.y = last_element->position.y + last_element->size.y + padding;
    }
    
    return element;
}

gui_element* possition_element_horizontally(gui_element* element, int padding, gui_element* last_element)
{
    if (element == NULL || last_element == NULL)
        return element;

    element->position.y = last_element->position.y;

    if (padding < 0)
    {
        padding++;

        element->position.x = last_element->position.x - element->size.x + padding;
    }
    else
    {
        element->position.x = last_element->position.x + last_element->size.x + padding;
    }
    
    return element;
}

void initialize_frame_element(gui_element* element)
{
    initialize_standered_element_values(element);

    element->draw = gui_standard_element_frame_draw_function;
}


void initialize_textbox_element(gui_element* element)
{
    initialize_standered_element_values(element);
    element->draw = gui_standard_element_frame_textbox_function;


    void* data = malloc(sizeof(gui_element_standard_element_text_box_data));

    if (data == NULL)
    {
        printf("[Error] Failed to initialize text box element, malloc failed\n");
        return;
    }

    memclr(data, sizeof(gui_element_standard_element_text_box_data));

    element->data = data;
}

void size_textbox_element(gui_element* element, int padding)
{
    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (data == NULL || data->str == NULL)
    {
        printf("[Error] Failed to size textbox element, no data struct or text\n");
        return;
    }


    uint32_t x = 0;
    uint32_t y = 0;
    
    display_get_text_size_px(data->str, &x, &y, UINT32_MAX, NULL);

    element->size.y = y + 2 * padding - 2;
    element->size.x = x + 2 * padding;
    data->offset.y = padding;
    data->offset.x = padding;
}

void center_text_element(gui_element* element, bool horizontal, bool vertical)
{
    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (data == NULL || data->str == NULL)
    {
        printf("[Error] Failed to size textbox element, no data struct or text\n");
        return;
    }


    uint32_t size_x = 0;         // Since this function uses text size, we need to calcuate it here
    uint32_t size_y = 0;
    display_get_text_size_px(data->str, &size_x, &size_y, UINT32_MAX, NULL);

    if (horizontal)
    {
        element->position.x -= size_x / 2;
    }

    if (vertical)
    {
        element->position.x += size_y / 2;
    }
}

void right_allign_text_element(gui_element* element)
{
    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (data == NULL || data->str == NULL)
    {
        printf("[Error] Failed to size textbox element, no data struct or text\n");
        return;
    }


    uint32_t size_x = 0;         // Since this function uses text size, we need to calcuate it here
    uint32_t size_y = 0;
    display_get_text_size_px(data->str, &size_x, &size_y, UINT32_MAX, NULL);

    element->position.x -= size_x;
}

gui_element* create_frame_element(dynamic_array* buffer)
{
    gui_element* element = create_element(buffer);

    if (!element)
        return NULL;

    initialize_frame_element(element);

    return element;
}

gui_element* create_text_element(const char* text, dynamic_array* buffer)
{
    gui_element* element = create_element(buffer);

    if (!element)
        return NULL;

    initialize_textbox_element(element);

    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (!data)
        return NULL;

    data->str = text;
    element->color_disabled.background = FRAMEBUFFER_RGBA(0, 0, 0, 255);
    element->color_focused.background = FRAMEBUFFER_RGBA(0, 0, 0, 255);
    element->color.background = FRAMEBUFFER_RGBA(0, 0, 0, 255);


    return element;
}

gui_element* create_textbox_element(const char* text, int padding, dynamic_array* buffer)
{
    gui_element* element = create_element(buffer);

    if (!element)
        return NULL;

    initialize_textbox_element(element);

    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (!data)
        return NULL;

    data->str = text;
    
    size_textbox_element(element, padding);

    return element;
}

void gui_standard_element_frame_draw_function(gui_element* element, gui_vec2 offset, int target_buffer)
{
    if (element->size.x == 0 || element->size.y == 0)
        return;

    int border_width = (element->flags & GUI_ELEMENT_FLAGS_FOCUSED) ? element->border_width_focused : element->border_width_normal;

    gui_element_colors* color = get_element_colors(element);

    int size_x = element->size.x - 1;
    int size_y = element->size.y - 1;
    
    draw_outline(element->position.x + offset.x, element->position.y + offset.y,
        element->position.x + size_x, element->position.y + size_y,
        border_width, color->border, target_buffer);

    display_fill_rect(element->position.x + border_width, element->position.y + border_width,
        element->position.x + size_x - (2 * border_width), element->position.y + size_y - (2 * border_width),
        color->background, target_buffer);
}

void gui_standard_element_frame_textbox_function(gui_element* element, gui_vec2 offset, int target_buffer)
{
    // First draw the background
    gui_standard_element_frame_draw_function(element, offset, target_buffer);

    // Now text
    gui_element_colors* color = get_element_colors(element);

    gui_element_standard_element_text_box_data* data = (gui_element_standard_element_text_box_data*)element->data;

    if (data == NULL || data->str == NULL)
        return;

    uint32_t x = element->position.x + data->offset.x + offset.x;
    uint32_t y = element->position.y + data->offset.y + offset.y;

    uint32_t width_with_padding = element->size.x - (2 * data->offset.x) + 2;
    uint32_t x_max =  (width_with_padding > element->size.x) ? UINT32_MAX : (width_with_padding + x);

    display_draw_string(data->str, &x, &y, x, x_max, true, NULL, color->forground, color->background, target_buffer);
}