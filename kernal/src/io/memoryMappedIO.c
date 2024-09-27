#include "io/memoryMappedIO.h"

size_t volatile  MMIO_Base_Address = 0;

size_t get_mmio_base_address(int boardType)
{
    switch (boardType) {
        case 1:     
            return 0x20000000; 
        case 2:     
            return 0x3F000000; 
        case 3:     
            return 0x3F000000; 
        case 4:    
            return 0xFE000000; 
        default:    
            return 0x20000000; 
    }
}

void set_mmio_base(int boardType)
{
    MMIO_Base_Address = get_mmio_base_address(boardType);
}