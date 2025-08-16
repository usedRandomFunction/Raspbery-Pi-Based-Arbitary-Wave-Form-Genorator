#include "gui/elements.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"





void draw_element(gui_element* element, int target_buffer)
{
    gui_vec2 offset;
    memclr(&offset, sizeof(gui_vec2));

    gui_element* parent = element;

    //Bassicly itterate of the parents of the element
    while ((parent = parent->parent))
    {
        offset.x += parent->position.x;
        offset.y += parent->position.y;
    }

    draw_element_recursive(element, offset, target_buffer);
}

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
