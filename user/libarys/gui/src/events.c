#include "gui/events.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/alloc.h"

#ifndef GUI_EVENT_QUEUE_DEFAULT_SIZE // THis is in entries -- TODO document this
#define GUI_EVENT_QUEUE_DEFAULT_SIZE 10
#endif

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
    memclr(queue, sizeof(gui_event_queue));

    queue->buffer = malloc(sizeof(gui_event*) * GUI_EVENT_QUEUE_DEFAULT_SIZE);

    if (!queue->buffer)
    {
        printf("[Error]: Unable to allocate memory for gui_event_queue! Failed to initialize.");
        return;
    }

    memclr(queue->buffer, sizeof(gui_event*) * GUI_EVENT_QUEUE_DEFAULT_SIZE);
    queue->buffer_size = GUI_EVENT_QUEUE_DEFAULT_SIZE;
    queue->write_index = 1;
}

void resize_gui_event_queue(gui_event_queue* queue, int number_of_entrys)
{
    // First check if we can do this resize
    int current_size = queue->write_index - queue->read_index;

    if (current_size < 0)
        current_size *= -1;

    if (number_of_entrys <= current_size)
        number_of_entrys = current_size;

    // Prepair the new buffer
    gui_event** new_buffer = (gui_event**)malloc(number_of_entrys * sizeof(gui_event*));

    if (!new_buffer)
    {
        printf("[Error]: Unable to allocate memory for gui_event_queue! Failed to resize\n");
        return;
    }

    memclr(new_buffer, sizeof(gui_event*) * number_of_entrys);
    // Now copy the data, and move it so it starts at index 0

    for (int i = 0; i < current_size; i++)
    {
        queue->read_index++;

        if (queue->read_index >= queue->buffer_size)
            queue->read_index = 0;
        
        new_buffer[i] = queue->buffer[queue->read_index];
    }

    // Set the read / write indexs
    queue->buffer_size = number_of_entrys;
    queue->write_index = current_size;
    queue->read_index = 0;

    // Now swap the buffers, 
    if (queue->buffer)
        free(queue->buffer);
    
    queue->buffer = new_buffer;
}

void free_gui_event_queue(gui_event_queue* queue)
{
    if (!queue)
        return;

    for (int i = 0; i < queue->buffer_size; i++)
        free_gui_event(queue->buffer[i]);
    
    if (queue->buffer)
        free(queue->buffer);
}

void gui_event_queue_push(gui_event_queue* queue, gui_event* event)  // TODO This might be a memory leak. Need to test it
{
    queue->buffer[queue->write_index] = event;

    // Get the next write index
    queue->write_index++;
    
    // Loop back
    if (queue->write_index >= queue->buffer_size)
        queue->write_index = 0;
    
    // Buffer full - resize.
    if (queue->write_index == queue->read_index)
    {
        queue->write_index = queue->buffer_size;
        resize_gui_event_queue(queue, queue->buffer_size + 1);
    }

}

gui_event* gui_event_queue_next(gui_event_queue* queue)
{
    if (queue->write_index - 1 == queue->read_index || 
        (queue->read_index == queue->buffer_size - 1 && queue->write_index == 0))    // Buffer only has one item (the one to remove) dont return anything
        return NULL;

    // First free last event
    free_gui_event(queue->buffer[queue->read_index]);
    queue->buffer[queue->read_index] = NULL;

    // Get next index
    queue->read_index++;
    
    // Loop back
    if (queue->read_index >= queue->buffer_size)
        queue->read_index = 0;

    return queue->buffer[queue->read_index];
}
