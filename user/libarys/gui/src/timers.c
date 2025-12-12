#include "gui/timers.h"
#include "gui/events.h"

#include "common/basic_io.h"
#include "common/memory.h"
#include "common/timing.h"
#include "common/alloc.h"

#include <stddef.h>

void initialize_gui_timer_queue(gui_timer_queue* queue)
{
	memclr(queue, sizeof(gui_timer_queue));
}

void resize_gui_timer_queue(gui_timer_queue* queue, int new_size)
{
	if (new_size < queue->number_of_elements)
        new_size = queue->number_of_elements;
    
    size_t      buffer_size_bytes   = new_size * sizeof(gui_timer*);
    gui_timer** new_buffer          = malloc(buffer_size_bytes);
    
    if (!new_buffer)
    {
        printf("[Error]: Unable to allocate memory for timer queue!");
        return;
    }

    memclr(new_buffer, buffer_size_bytes);

    size_t copy_size = queue->number_of_elements * sizeof(gui_timer*);
    memcpy(new_buffer, queue->buffer + queue->read_index, copy_size);

    free(queue->buffer);
    queue->buffer_size = new_size;
    queue->buffer = new_buffer;

}


void free_gui_timer_queue(gui_timer_queue* queue)
{
    for (int i = 0; i < queue->buffer_size; i++)
    {
        if (queue->buffer[i])
            free(queue->buffer[i]);
    }
}

void gui_timer_queue_push(gui_timer_queue* queue, gui_timer* timer)
{
    gui_timer** working_buffer = queue->buffer;
    gui_timer** source_buffer = queue->buffer + queue->read_index;
    
    // Allocate nwe buffer if needed
    if (queue->number_of_elements + 1 > queue->buffer_size)
    {
        int     new_size            = queue->number_of_elements + 1;
        size_t  buffer_size_bytes   = new_size * sizeof(gui_timer*);
        working_buffer  = malloc(buffer_size_bytes);

        if (!working_buffer)
        {
            printf("[Error]: Unable to allocate memory for timer queue!\n"
                   "       - Unable to push timer (ID: %X)", timer->id);
            free(timer);
            return;
        }
 
        memclr(working_buffer, buffer_size_bytes);
        queue->buffer_size = new_size;
        queue->buffer = working_buffer;
    }
    
    // Copy elements
    for (int i = 0; i < queue->number_of_elements; i++)
    {
        gui_timer* current_timer = *source_buffer++;                // Get the current element and itterate the pointer 

        if (timer && timer->end_count < current_timer->end_count)   // If the timer to insert goes before this element 
        {                                                           // insert it here. But only if the element has not allready been inserted
            *working_buffer++ = timer;                              // Write and itterate
            timer = NULL;
        }

        *working_buffer++ = current_timer;                          // Write and itterate
    }
    
    if (timer)                                                      // If we have not allready inserted the timer, add it
        *working_buffer = timer;                                    // Should only run when inserting the first timer to the queue


    queue->number_of_elements++;
    queue->read_index = 0;
}

void gui_timer_queue_pop(gui_timer_queue* queue)
{
    if (queue->read_index == queue->buffer_size) 
        return;              // Queue empty - Nothing to pop
    
    queue->buffer[queue->read_index++] = 0;
    queue->number_of_elements--;
}


gui_timer* gui_timer_queue_peek(gui_timer_queue* queue)
{
	if (queue->read_index == queue->buffer_size) 
        return NULL;        // Empty dont return anything

    return queue->buffer[queue->read_index];
}


void gui_timer_queue_create_future_timer(gui_timer_queue* queue, uint64_t microseconds, uint64_t id)
{
    uint64_t timer_freqency = (uint64_t)get_timer_frequency();
    uint64_t start_time = get_timer_count();
    
    // F is in Hz (10^0) but T is in us (10^-6) FT = c, so F / 10^6 * T works for our units
    uint64_t delta = (timer_freqency / 1000000) * microseconds;
    uint64_t end_time = start_time + delta;

    gui_timer* timer = malloc(sizeof(gui_timer));
    
    if (!timer)
    {
        printf("[Error]: Unable to allocate memory for timer!");
        return;
    }

    memclr(timer, sizeof(gui_timer));
    
    timer->start_count = start_time;
    timer->end_count = end_time;
    timer->id = id;
    
    gui_timer_queue_push(queue, timer);
}


gui_event* gui_timer_check_for_timer_events(gui_timer_queue* queue)
{
    gui_timer* next_timer = gui_timer_queue_peek(queue);
    uint64_t current_time = get_timer_count();

    if (!next_timer)                            // If there is no timer then no checks are needed
        return NULL;

    if (next_timer->end_count > current_time)   // Timer not triggered, no event 
        return  NULL;

    if (next_timer->start_count > next_timer->end_count && next_timer->start_count > current_time) // Timer value has looped around. Dont trigger yet.
        return  NULL;

    gui_event* event = malloc(sizeof(gui_event));

    if (!event)
    {
        printf("[Error]: Failed to create event for gui_timer - %X\n"
               "         Unable to allocate memory for event.");
        return NULL;                            // We still keep the timer object in case it successeds in a later itteration
    }
    
    memclr(event, sizeof(gui_event));
    event->event_metadata = GUI_EVENT_METADATA_CAN_FREE_EVENT_DATA;
    event->event_type = GUI_EVENT_TYPE_TIMER_TRIGGERED;
    event->event_data = next_timer;

    gui_timer_queue_pop(queue);

    return event;
} 
