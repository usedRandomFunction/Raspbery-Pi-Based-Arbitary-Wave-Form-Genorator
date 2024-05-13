#ifndef ARM_EXCEPTIONS_H
#define ARM_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

void kernel_panic();

void arm_exception_handler(unsigned long type);

#ifdef __cplusplus
}
#endif

#endif