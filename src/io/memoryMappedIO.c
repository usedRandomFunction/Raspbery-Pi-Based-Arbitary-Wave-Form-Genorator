#include "io/memoryMappedIO.h"

size_t volatile  MMIO_Base_Address = 0;

void set_mmio_base(int boardType)
{
    switch (boardType) {
        case 1:     
            MMIO_Base_Address = 0x20000000; 
            break;
        case 2:     
            MMIO_Base_Address = 0x3F000000; 
            break;
        case 3:     
            MMIO_Base_Address = 0x3F000000; 
            break;
        case 4:    
            MMIO_Base_Address = 0xFE000000; 
            break;
        default:    
            MMIO_Base_Address = 0x20000000; 
            break;
    }
}