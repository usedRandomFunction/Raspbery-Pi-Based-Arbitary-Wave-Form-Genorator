#ifndef INTERPUTS_H
#define INTERPUTS_H

#include <stdbool.h>

typedef void (*USER_INTERUPT_HANDLER)(void);

// Used by some handler functions to check
// if they where called from a interupt or
// dirrectly
extern bool interupt_active;

// Dissables all interupts incase they have not defulted to zero
// and sets gpu irq routing
void initialize_interupts();

// Enables the given IRQ
// @param id the IRQ to enable
void enable_irq(int id);

// Disables the given IRQ
// @param id the IRQ to disable
void disable_irq(int id);

// Sets what CPU core gets the GPU interupts
// @param core [0, 3] inclusive
void route_gpu_irqs(int core);

// Used by vectors.S to handle interupts
void generic_irq_handler();

// Stores the address of the handler, at the possition of the given ID
// @param handler Pointer to handler function (in user space)
// @param id The ID of the user interupt handler
void register_user_interupt_handler(USER_INTERUPT_HANDLER handler, int id);

// Remove the hanlder at the given ID
// (Identical to register_user_interupt_handler((USER_INTERUPT_HANDLER)POINTER_MAX, id))
void remove_user_interupt_handler(int id);

// Runs the given interupt handler function
// @param id The ID of the handler function to run
// @note If the function is set to POINTER_MAX, a error will be printed and the operation will be aborted
void trigger_user_interupt_handler(int id);


#endif