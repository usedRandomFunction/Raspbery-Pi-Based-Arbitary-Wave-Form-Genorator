#ifndef EVENTS_H
#define EVENTS_H

// In the context of this OS a "event" is a function that will call other functions,
// however what functions (If any) that will be called changes during runtime.
// Their main use is for prg_exit, so it won't skip the end of interupt handling code

typedef void (*STANDERED_EVENT_HANDLER)(void);

// Ensures all memory associated with the 
// event handler is zeroed.
void initialize_event_handler();

// Adds a handler to be run when the `interupt_end` event occurs
// @param handler Pointer to handler function
void event_handler_add_interupt_end(STANDERED_EVENT_HANDLER handler);

// Runs all event handlers associated with the `interupt_end` event
void event_handler_on_interupt_end();


#endif