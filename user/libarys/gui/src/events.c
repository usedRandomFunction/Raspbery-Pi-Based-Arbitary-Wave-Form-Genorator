#include "gui/events.h"

#include "common/memory.h"
#include "common/alloc.h"
#include "common/math.h"

void free_gui_event(gui_event* event)
{
    if (!event)
        return;

    if (event->event_data && (event->event_metadata & GUI_EVENT_METADATA_CAN_FREE_EVENT_DATA))
        free(event->event_data);

    free(event);
}

void initialize_gui_event_queue(gui_event_queue* queue)
{
    initialize_dynamic_array(sizeof(gui_event*), 0, &queue->buffer);
    queue->write_index = -1;
    queue->read_index = -1;
}

void resize_gui_event_queue(gui_event_queue* queue, int number_of_entrys)
{
    // First check if we can do this resize
    int current_size = queue->write_index - queue->read_index;

    if (current_size < 0)
        current_size *= -1;

    if (number_of_entrys <= current_size)
        return;

    // Now preapre a buffer
    dynamic_array tempary_buffer_header;
    initialize_dynamic_array(sizeof(gui_event*), number_of_entrys, &tempary_buffer_header);
    memclr(tempary_buffer_header.ptr, sizeof(gui_event*) * number_of_entrys);

    // And cast it
    gui_event** tempary_buffer = (gui_event**)tempary_buffer_header.ptr;
    gui_event** currnet_buffer = (gui_event**)queue->buffer.ptr;

    // Now copy the data, and move it so it starts at index 0

    int i = 0;
    for ( ; i < current_size; i++)
    {
        queue->read_index++;

        if (queue->read_index >= queue->buffer.number_of_entrys)
            queue->read_index = 0;

        tempary_buffer[i] = currnet_buffer[queue->read_index];
    }

    // Set the read / write indexs
    queue->write_index = current_size - 2;
    queue->read_index = -1;

    // Now swap the buffers, 
    delete_dynamic_array(&queue->buffer);
    memcpy(&queue->buffer, &tempary_buffer_header, sizeof(dynamic_array));
}

void free_gui_event_queue(gui_event_queue* queue)
{
    if (!queue)
        return;

    gui_event** buffer = (gui_event**)queue->buffer.ptr;
    for (int i = 0; i < queue->buffer.number_of_entrys; i++)
        free_gui_event(buffer[i]);

    delete_dynamic_array(&queue->buffer);
}

void gui_event_queue_push(gui_event_queue* queue, gui_event* event)
{
    // First get the write index

    if (queue->write_index == queue->read_index)    // If we will overwrite allocate more space
    {
        resize_gui_event_queue(queue, queue->buffer.number_of_entrys + 1);
    } 
    
    queue->write_index++;

    if (queue->write_index >= queue->buffer.number_of_entrys)
    {
        if (queue->read_index == -1) // I.e we are going to overwrite a event
            resize_gui_event_queue(queue, queue->buffer.number_of_entrys + 1);

        queue->write_index = 0;
    }
    
    gui_event** buffer = (gui_event**)queue->buffer.ptr;
    buffer[queue->write_index] = event;
}

gui_event* gui_event_queue_next(gui_event_queue* queue)
{
    if (queue->write_index == queue->read_index)    // Buffer is empty return nothing
        return NULL;

    gui_event** buffer = (gui_event**)queue->buffer.ptr;

    // First free last event
    if (queue->read_index >= 0)
    {
        free_gui_event(buffer[queue->read_index]);
        buffer[queue->read_index] = NULL;
    }

    // Get next index
    queue->read_index++;

    if (queue->read_index >= queue->buffer.number_of_entrys)
        queue->read_index = 0;

    return buffer[queue->read_index];
}
