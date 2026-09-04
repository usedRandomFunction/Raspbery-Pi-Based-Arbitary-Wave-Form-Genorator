#include "common/math.h"

double pow(double a, double b)
{
    if (b == 0)
        return 1;

    if (b < 0)
        return 1 / pow(a, -b);

    const double base = a;
    a = 1;

    for ( ; b > 0; b--)
        a *= base;

    return a;
}

int powl(int base, unsigned int exp)
{
    int a = 1;
    if (exp == 0)
        return 1;

    for ( ; exp != 0U - 1U; exp--)
        a *= base;

    return a;
}

unsigned int powul(unsigned int base, unsigned int exp)
{
    unsigned int a = 1;

    if (exp == 0)
        return 1;

    for ( ; exp != 0U - 1U; exp--)
        a *= base;

    return a;
}

double fmod(double a, double b)
{
    return a - floor(a/b) * b;
}

double fabs(double x)
{
    if (x < 0)
        return -x;

    return x;
}

double floor(double x)
{
    return __builtin_floor(x);
}

double ceil(double x)
{
    return __builtin_floor(x);
}

double round(double x)
{
    return __builtin_round(x);
}

double round_to(double x, int mag)
{
    double multiplier = pow(10, mag);

    return multiplier * round(x / multiplier);
}

double abs(double x)
{
    return x >= 0 ? x : -x;
}


double log10(double x)
{
    return ln(x) / M_Nat_log_10;
}

// Realtivly simple apporximation of ln, 
// https://stackoverflow.com/questions/9799041/efficient-implementation-of-natural-logarithm-ln-and-exponentiation
double ln(double y)
{
    int log2 = 0;
    double divisor, x, result;
    
    int bits = (int)y;
    while (bits >>= 1) { log2++; } // Gets the number of bits before the MSB
    //log2 = __builtin_ctz((int)y); // See: https://stackoverflow.com/a/4970859/6630230

    divisor = (double)(1 << log2);
    x = y / divisor;    // normalized value between [1.0, 2.0]

    result = -1.7417939 + (2.8212026 + (-1.4699568 + (0.44717955 - 0.056570851 * x) * x) * x) * x;
    result += ((float)log2) * 0.69314718; // ln(2) = 0.69314718

    return result;
}

// Ok i couldn't find a license for this one but it came form
// https://gist.github.com/giangnguyen2412/bcab883b5a53b437b980d7be9745beaf
// 'compare_float', 'cos' and 'sin'

int compare_float(double f1, double f2)
{
    double precision = 0.00000000000000000001;
    if ((f1 - precision) < f2)
    {
        return -1;
    }
    else if ((f1 + precision) > f2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

double cos(double x)
{
    if( x < 0.0f ) 
        x = -x;

    if (0 <= compare_float(x,M_PI_M_2)) 
    {
        do 
        {
            x -= M_PI_M_2;
        }
        while(0 <= compare_float(x,M_PI_M_2));
    }

    if ((0 <= compare_float(x, M_PI)) && (-1 == compare_float(x, M_PI_M_2)))
    {
        x -= M_PI;
        return ((-1)*(1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))))));
    } 
    return 1.0f - (x*x/2.0f)*( 1.0f - (x*x/12.0f) * ( 1.0f - (x*x/30.0f) * (1.0f - (x*x/56.0f )*(1.0f - (x*x/90.0f)*(1.0f - (x*x/132.0f)*(1.0f - (x*x/182.0f)))))));
}

double sin(double x)
{
    return cos(x-M_PI_2);
}
