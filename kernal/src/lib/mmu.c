#include "lib/mmu.h"

#include "lib/arm_exceptions.h"
#include "lib/memory.h"
#include "io/printf.h"


void write_page_descriptor(uint64_t* descriptor_address, void* pointer_address, uint64_t attributes, bool block_table_bit)
{
    size_t pointer_address_as_number = (size_t)pointer_address;
    uint64_t descriptor = 0b1; // Enable the valid bit

    if (pointer_address_as_number & 0xFFFF000000000FFF)
    {
        printf("write_page_descriptor: failed!\npointer_address has bits set outside of [47:12]!\n%x\n", pointer_address);
        kernel_panic();
    }

    if (block_table_bit == true)
        descriptor |= 0b10;

    descriptor |= (uint64_t)pointer_address_as_number;
    descriptor |= (uint64_t)(attributes); // TODO maby add some bitwise stuff to ensure no bad attributes

    *descriptor_address = descriptor;
}

void print_page_descriptor(uint64_t* descriptor_address)
{
    uint64_t descriptor = *descriptor_address;
    printf("Page table entry: %x: \n    valid: ", get_physical_address(descriptor_address));

    if (descriptor & 0b1)
    {
        printf("true\n    type: ");
    }
    else
    {
        printf("false\n");
        return;
    }

    if (descriptor & 0b10)
    {
        printf("table entry\n");
    }
    else
    {
        printf("page address\n");
    }

    printf("    pointer: %x\n    attributes: %x\n", 
        descriptor & 0xFFFFFFFFF000,
        descriptor & (~0xFFFFFFFFF000));
}

void set_ttbr1_el1(void* ptr)
{
    asm volatile ("msr ttbr1_el1, %0"
	:
	: "r" (get_physical_address(ptr)));
    invalidate_tlb();
}

void set_ttbr0_el1(void* ptr)
{   
    asm volatile ("msr ttbr0_el1, %0"
	:
	: "r" (get_physical_address(ptr)));
    invalidate_tlb();
}   

void* get_ttbr0_el1()
{   
    uint64_t ttbr0;
	asm volatile ("mrs %x0, ttbr0_el1" : "=r" (ttbr0));
    
    return (void*)ttbr0;
}  

void invalidate_tlb()
{
    asm volatile (
    "tlbi vmalle1is\n\t"  // Invalidate all TLB entries for EL1
    "dsb sy\n\t"                       // Data Synchronization Barrier
    "isb");                             // Instruction Synchronization Barrier
}

void enable_caches()
{
    size_t sctlr;
    asm volatile ("mrs %x0, sctlr_el1" : "=r" (sctlr));

    sctlr |= 1 << 2;                    // Set the C bit (bit 2) to enable data cache
    sctlr |= 1 << 12;                   // Set the I bit (bit 12) to enable instruction cache

    asm volatile ("msr sctlr_el1, x0\n\t"
    "isb"                               // Instruction Synchronization Barrier
	:
	: "r" (sctlr)
	: "x0");

    invalidate_tlb();
}

void dissable_caches()
{
    size_t sctlr;
    asm volatile ("mrs %x0, sctlr_el1" : "=r" (sctlr));

    sctlr &= ~(((size_t)1) << 2);       // Reset the C bit (bit 2) to dissable data cache
    sctlr &= ~(((size_t)1) << 12);      // Reset the I bit (bit 12) to dissable instruction cache

    asm volatile ("msr sctlr_el1, x0\n\t"
    "isb"                               // Instruction Synchronization Barrier
	:
	: "r" (sctlr)
	: "x0");
    
    invalidate_tlb();
}

void invalidate_data_cache(void* VA_start, void* VA_end)
{

    uint64_t csselr_el1;
    asm volatile ("mov x0, xzr\n\t"     // zero means get L1 data cache
        "msr csselr_el1, x0\n\t"
        "mrs %x0, csselr_el1" : "=r" (csselr_el1) : : "x0");

    csselr_el1 &= 0x7;                  // Get The bytes per (cache) line
    csselr_el1 += 4;

    ptrdiff_t bytes_per_line = 1 << csselr_el1;

    for (uint8_t* VA = (uint8_t*)VA_start; (void*)VA < VA_end; VA += bytes_per_line)
    {
        asm volatile ("dc civac, %0" : : "r" (VA));
    }

    asm volatile ("dsb sy\n\t"
        "isb");
}

void invalidate_data_cache_of_size(void* VA_start, size_t size)
{
    invalidate_data_cache(VA_start, void_ptr_offset_bytes(VA_start, size));
}