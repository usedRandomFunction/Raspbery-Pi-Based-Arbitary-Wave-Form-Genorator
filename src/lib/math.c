#include "lib/math.h"



int pow(int base, unsigned int exp)
{
    if (exp = 0)
        return 1;

    for ( ; exp != 0U - 1U; exp--)
        base *= base;

    return base;
}

unsigned int powu(unsigned int base, unsigned int exp)
{
    if (exp = 0)
        return 1;

    for ( ; exp != 0U - 1U; exp--)
        base *= base;

    return base;
}