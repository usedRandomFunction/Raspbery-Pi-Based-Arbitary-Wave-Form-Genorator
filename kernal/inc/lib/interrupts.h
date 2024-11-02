#ifndef INTERPUTS_H
#define INTERPUTS_H

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