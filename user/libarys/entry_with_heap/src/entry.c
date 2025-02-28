#include "central_block_memory_allocator/central_block_memory_allocator.h"
#include "common/program_managment.h"
#include "common/basic_io.h"
#include "common/memory.h"

int main();

extern char __bss_start[];
extern char __bss_end[];
extern char __heap_start[];
extern char __heap_end[];

extern 

void _entry() __attribute__((section(".start")));

void _create_bss();

void _initialize_heap();

central_block_memory_allocator_header _defult_allocator;

void _entry()
{
    _create_bss();
    _initialize_heap();


    exit(main());
}

void _create_bss()
{
    const size_t bss_size = (size_t)__bss_end - (size_t)__bss_start;

    if (bss_size == 0)
        return;

    if (vmemmap(&__bss_start, bss_size, VMEMMAP_WRITABILITY) < bss_size)
        exit(-9999);
        
    memclr(&__bss_start, bss_size);
}

void _initialize_heap()
{
    void* heap_start = &__heap_start;
    void* heap_end = &__heap_end;

    size_t minimum_size = (size_t)heap_end - (size_t)heap_start;

    if (minimum_size == 0)
        return;

    size_t real_size = vmemmap(heap_start, minimum_size, VMEMMAP_WRITABILITY);

    if (real_size < minimum_size)
        exit(-10000);

    initialize_central_block_memory_allocator(heap_start, real_size, 5, &_defult_allocator);
}

void free(void* p)
{
    if (p == NULL)
    {
        printf("Attempted to free NULL pointer\nCaller: 0x%x\n", __builtin_return_address(0));
        exit(-10001);
    }

    central_block_memory_allocator_free(p, &_defult_allocator);
}

void* malloc(size_t size)
{
    if (size == 0)
    {
        printf("Attempted to allocate zero bytes\nCaller: 0x%x\n", __builtin_return_address(0));
        exit(-10001);
    }

    return central_block_memory_allocator_alloc(size, &_defult_allocator);
}

void* aligned_alloc(size_t alignment, size_t size)
{
    if (size == 0)
    {
        printf("Attempted to allocate zero bytes\nCaller: 0x%x\n", __builtin_return_address(0));
        exit(-10001);
    }

    if (alignment != 0)
    {
        if (((alignment & (alignment - 1)) != 0))
        {
            printf("\nUnable to allocate memory, only alight ments of a power of two\n");
            return NULL;
        }

        for (int i = 0; i < sizeof(size_t)*8; i++)
        {
            if (alignment & 0b1)
            {
                alignment = i;
                break;
            }

            alignment >>= 1;
        }
    }
    
    return central_block_memory_allocator_alloc_alligned(size, alignment, &_defult_allocator);
}