#ifndef INTERPUTS_H
#define INTERPUTS_H

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

#endif