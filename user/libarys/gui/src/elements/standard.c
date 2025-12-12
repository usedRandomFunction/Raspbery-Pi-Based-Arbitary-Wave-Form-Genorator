#include "gui/elements/standard.h"
#include "gui/drawing_functions.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"


void initialize_frame_element(gui_element* element)
{
    initialize_standered_element_values(element);

    element->draw = gui_standard_element_frame_draw_function;
}

void initialize_textbox_element(gui_element* element)
{
    initialize_standered_element_values(element);
    element->draw = gui_standard_element_textbox_draw_function;


    void* data = malloc(sizeof(gui_standard_element_text_box_data));

    if (data == NULL)
    {
        printf("[Error] Failed to initialize text box element, malloc failed\n");
        return;
    }

    memclr(data, sizeof(gui_standard_element_text_box_data));

    element->data = data;
}

void size_textbox_element(gui_element* element, int padding)
{
    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

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

void size_textbox_element_for_n_characters(gui_element *element, int n, int padding)
{
    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

    if (data == NULL)
    {
        printf("[Error] Failed to size textbox element, no data struct\n");
        return;
    }

    uint32_t x = 0;
    uint32_t y = 0;

    const char* current_string_ptr = data->str;
    data->str = "X"; // Just needs to be a singal charecter string
    
    // Get the size of it, i mean we use monospace fonts
    display_get_text_size_px(data->str, &x, &y, UINT32_MAX, NULL);
    
    // And use that size to create the required padding
    element->size.y = y + 2 * padding - 2;
    element->size.x = x * n + 2 * padding;
    data->offset.y = padding;
    data->offset.x = padding;
    data->str = current_string_ptr;
}

void center_text_element(gui_element* element, bool horizontal, bool vertical)
{
    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

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
    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

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

    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

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

    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

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
        element->position.x + size_x + offset.x, element->position.y + size_y + offset.y,
        border_width, color->border, target_buffer);

    display_fill_rect(element->position.x + border_width + offset.x, element->position.y + border_width + offset.y,
        element->position.x + size_x - (2 * border_width) + offset.x, element->position.y + size_y - (2 * border_width) + offset.y,
        color->background, target_buffer);
}

void gui_standard_element_textbox_draw_function(gui_element* element, gui_vec2 offset, int target_buffer)
{
    // First draw the background
    gui_standard_element_frame_draw_function(element, offset, target_buffer);

    // Now text
    gui_element_colors* color = get_element_colors(element);

    gui_standard_element_text_box_data* data = (gui_standard_element_text_box_data*)element->data;

    if (data == NULL)
        return;

    uint32_t x = element->position.x + data->offset.x + offset.x;
    uint32_t y = element->position.y + data->offset.y + offset.y;

    uint32_t width_with_padding = element->size.x - (2 * data->offset.x) + 2;
    uint32_t x_max =  (width_with_padding > element->size.x) ? UINT32_MAX : (width_with_padding + x);
    
    if (data->str != NULL)
        display_draw_string(data->str, &x, &y, x, x_max, true, NULL, color->forground, color->background, target_buffer);
    
    // Now cursor (If needed and posible)
    
    if (!data->cursor_visible || x >= x_max) 
        return;

    display_draw_string(" ", &x, &y, x, x_max, true, NULL, color->background, color->forground, target_buffer);
}
