#include "lib/mmu.h"

#include "lib/arm_exceptions.h"
#include "io/uart.h"


void write_page_descriptor(uint64_t* descriptor_address, void* pointer_address, uint16_t upper_attributes, uint16_t lower_attributes, bool block_table_bit)
{
    size_t pointer_address_as_number = *((size_t*)&pointer_address);
    uint64_t descriptor = 0b1; // Enable the valid bit

    if (pointer_address_as_number & 0xFFFF000000000FFF)
    {
        uart_puts("write_page_descriptor: failed!\npointer_address has bits set outside of [47:12]!\n");
        uart_put_ptr(pointer_address);
        kernel_panic();
    }

    if (block_table_bit == true)
        descriptor |= 0b10;

    descriptor |= (uint64_t)(lower_attributes & 0b111111111) << 2;
    descriptor |= (uint64_t)pointer_address_as_number;
    descriptor |= (uint64_t)((upper_attributes & 0x3FFFFF)) << 48;

    *descriptor_address = descriptor;
}

void set_ttbr1_el1(void* ptr)
{
    asm volatile ("msr ttbr1_el1, x0"
	:
	: "r" (ptr)
	: "x0");
    invalidate_tlb();
}

void set_ttbr0_el1(void* ptr)
{   
    asm volatile ("msr ttbr0_el1, x0"
	:
	: "r" (ptr)
	: "x0");
    invalidate_tlb();
}   

void invalidate_tlb()
{
    asm volatile ("tlbi vmalle1is\n\t"  // Invalidate all TLB entries for EL1
    "dsb ish\n\t"                       // Data Synchronization Barrier
    "isb");                             // Instruction Synchronization Barrier
}

void enable_caches()
{
    invalidate_caches();
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

    sctlr &= ~(((size_t)1) << 2);                    // Reset the C bit (bit 2) to dissable data cache
    sctlr &= ~(((size_t)1) << 12);                   // Reset the I bit (bit 12) to dissable instruction cache

    asm volatile ("msr sctlr_el1, x0\n\t"
    "isb"                               // Instruction Synchronization Barrier
	:
	: "r" (sctlr)
	: "x0");
    
    invalidate_tlb();
}

void invalidate_caches()
{
    asm volatile ("ic iallu\n\t"        // Invalidate all instruction caches to PoU
    "dsb nsh\n\t"                       // Data Synchronization Barrier
    "isb\n\t"                           // Instruction Synchronization Barrier
    "dc ivac, xzr\n\t"                  // Invalidate all data caches to PoU
    "dsb nsh");                         // Data Synchronization Barrier
}