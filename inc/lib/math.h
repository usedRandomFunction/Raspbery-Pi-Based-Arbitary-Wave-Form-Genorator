#ifndef MATH_H
#define MATH_H

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
#define clamp(a, ma, mi) max(min(a, ma), mi)

#ifdef __cplusplus
extern "C" {
#endif

int pow(int base, unsigned int exp);

unsigned int powu(unsigned int base, unsigned int exp);

#ifdef __cplusplus
}
#endif

#endif