#ifndef LIBGUI_TIMERS_H
#define LIBGUI_TIMERS_H

#include <stdint.h>

// This class is used to allow a timer to pass a gui event
// It should be noted that the timer is polled, not interupt.
// The event is also appeneded at the end of the queue
struct gui_timer
{
    uint64_t start_count;   // The value of cntpct_el0 when the timer started
    uint64_t end_count;     // The value of cntpct_el0 when the timer will end
    uint64_t id;            // User given ID used to idntify the timer
};

typedef struct gui_timer gui_timer;

// Stores a ordered (first to trigger first) queue of the gui_timers
// Internally the elements are linear, however when a timer is removed 
// the read_index index increases. This stops uneeded memcpys 
// When a element is added the array is moved to index 0 and element 
// will be inserted at the correct index during this operation
// this queue is heap allocated, and will grow if needed
// TODO Since the buffer can grow, it should probibly be able to shrink
struct gui_timer_queue 
{
    gui_timer** buffer;      // Pointer to the memory buffer its self
    int number_of_elements; // The number of gui_timer's stored in this object
    int buffer_size;        // The size of the buffer in entires
    int read_index;         // Stores the index the next element is stored at,
};

typedef struct gui_timer_queue gui_timer_queue;

// Initialize the queue
// @param buffer Queue to initialize
void initialize_gui_timer_queue(gui_timer_queue* queue);

// Resizes the queue to store `number_of_entrys`
// @param queue Queue to resize
// @param new_size New size
// @note If resizing will delete entires the new size will be the minimum size to store all entires 
void resize_gui_timer_queue(gui_timer_queue* queue, int new_size);

// Frees the buffer, and all timers the buffer points to
// @param queue Queue to free
// @note This will not attempt to free the gui_timer_queue header its self
void free_gui_timer_queue(gui_timer_queue* queue);

// Pushes a event to the back of the queue
// @param queue The queue to push onto
// @param timer The timer to push
// @note timere Must be heap allocated, not on the stack, the pointer its self will be copyed to the queue, the memory will be freed later by the queue
void gui_timer_queue_push(gui_timer_queue* queue, gui_timer* timer);

// Removes the first item from the queue and does not free it 
// @param queue The queue to pop an item from 
// @warning This function is intented for internal use, but is exposed to allow for greater useablity
void gui_timer_queue_pop(gui_timer_queue* queue);

// Takes the first item from the queue and returns it, 
// the item is left inside the queue
// @param queue Queue to veiw first item from 
// @return A pointer to the timer or NULL if no item is present 
// @warning This function is intented for internal use, but is exposed to allow for greater useablity
gui_timer* gui_timer_queue_peek(gui_timer_queue* queue);

// Creates and push a new timer event 
// @param queue Queue to push to 
// @param microseconds How many microseconds in the future should the event trigger
// @param id The ID of the timer event
// @note Timers 0x2000 to 0x1000 are reserved for internal use by the GUI libary
void gui_timer_queue_create_future_timer(gui_timer_queue* queue, uint64_t microseconds, uint64_t id);

// Checks the closest timer, if end_count has been reached it returns its event and clears it from the queue 
// @param queue Queue to check closest timer from 
// @returns pointer to gui_event if timer has exprired or NULL if it as not.
struct gui_event* gui_timer_check_for_timer_events(gui_timer_queue* queue);  

#endif
