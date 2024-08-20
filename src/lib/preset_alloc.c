#include "lib/preset_alloc.h"
#include "io/printf.h"

#include <stdbool.h>

static void* buffer_A;
static void* buffer_B;
static bool  buffer_A_Used;
static bool  buffer_B_Used;
static size_t buffer_A_Length;
static size_t buffer_B_Length;

void preset_alloc_set_buffers(void* A, size_t len_A, void* B, size_t len_B)
{
    buffer_A = A;
    buffer_B = B;
    buffer_A_Used = false;
    buffer_B_Used = false;
    buffer_A_Length = len_A;
    buffer_B_Length = len_B;
}

void* preset_alloc_aligned_alloc(size_t alignment, size_t size)
{
    if (buffer_A_Used == false &&
        size == buffer_A_Length && (alignment == 0 || (*(size_t*)&buffer_A % alignment == 0)))
    {
        buffer_A_Used = true;
        return buffer_A;
    }
    else if (buffer_B_Used == false &&
        size == buffer_B_Length && (alignment == 0 || (*(size_t*)&buffer_B % alignment == 0)))
    {
        buffer_B_Used = true;
        return buffer_B;
    }

    printf("preset_alloc failed!\n");
}

void preset_alloc_free(void* p)
{
    if (p == buffer_A)
        buffer_A_Used = false;
    else if (p == buffer_B)
        buffer_B_Used = false;
}