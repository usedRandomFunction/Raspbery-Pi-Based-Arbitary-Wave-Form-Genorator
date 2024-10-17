#ifndef MATH_H
#define MATH_H

#define M_PI 3.14159265358979323846
#define M_PI_2 (M_PI / 2.0)
#define M_PI_M_2 (M_PI * 2.0)

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
#define clamp(a, ma, mi) max(min(a, ma), mi)

#define rotate_right(result, value, shift) asm ("ror %0, %1, %2" : "=r" (result) : "r" (value), "r" (shift));
#define rotate_left(result, value, shift) asm ("ror %0, %1, %2" : "=r" (result) : "r" (value), "r" (64 - (shift)));




// Calcautes base to the power of exp
// @param base The base of the power
// @param exp The power its self
// @return base to the power of exp
int pow(int base, unsigned int exp);

// Calcautes base to the power of exp
// @param base The base of the power
// @param exp The power its self
// @return base to the power of exp
unsigned int powu(unsigned int base, unsigned int exp);


// Taylor series cosine function for floating point numbers
// @param x Angle in radians
double cos(double x);

// Taylor series sine function for floating point numbers
// @param x Angle in radians
double sin(double x);



#endif