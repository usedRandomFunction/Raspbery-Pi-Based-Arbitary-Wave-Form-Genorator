#include "boot/boardDectetion.h"
#include <stdint.h>

int get_board_type()
{
    uint32_t reg;
	#ifdef AARCH64
		asm volatile ("mrs %x0, midr_el1" : "=r" (reg));
	#else
		asm volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (reg));
	#endif

    switch ((reg >> 4) & 0xFFF) {
        case 0xB76: return 1; 
        case 0xC07: return 2; 
        case 0xD03: return 3; 
        case 0xD08: return 4; 
        default:   return -1;
    }
}

const char* get_board_name(int boardType)
{
    switch (boardType) {
        case 1:     return "Rpi(1/0)";
        case 2:     return "Rpi2";
        case 3:     return "Rpi3";
        case 4:     return "Rpi4";
        default:    return "????";
    }
}