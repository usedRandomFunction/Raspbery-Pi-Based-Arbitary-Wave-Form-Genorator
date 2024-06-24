// Adapted from c header to asm 
// Origal avalible at https://github.com/s-matyukevich/raspberry-pi-os/blob/master/src/lesson06/include/mm.h
// And https://github.com/s-matyukevich/raspberry-pi-os/blob/master/src/lesson06/include/arm/mmu.h

.equ VA_START, 			        0xffff000000000000

.equ PHYS_MEMORY_SIZE, 		    0x40000000	

.equ PAGE_MASK,			        0xfffffffffffff000
.equ PAGE_SHIFT,	 		    12
.equ TABLE_SHIFT, 			    9
.equ SECTION_SHIFT,			    (PAGE_SHIFT + TABLE_SHIFT)

.equ PAGE_SIZE,   			    (1 << PAGE_SHIFT)	
.equ SECTION_SIZE,			    (1 << SECTION_SHIFT)	

.equ LOW_MEMORY,              	(2 * SECTION_SIZE)

.equ PTRS_PER_TABLE,			(1 << TABLE_SHIFT)

.equ PGD_SHIFT,			        PAGE_SHIFT + 3*TABLE_SHIFT
.equ PUD_SHIFT,			        PAGE_SHIFT + 2*TABLE_SHIFT
.equ PMD_SHIFT,			        PAGE_SHIFT + TABLE_SHIFT

.equ PG_DIR_SIZE,			    (3 * PAGE_SIZE)

.equ MM_TYPE_PAGE_TABLE,		0x3
.equ MM_TYPE_PAGE, 			    0x3
.equ MM_TYPE_BLOCK,		    	0x1
.equ MM_ACCESS,			        (0x1 << 10)
.equ MM_ACCESS_PERMISSION,		(0x01 << 6) 

/*
 * Memory region attributes:
 *
 *   n = AttrIndx[2:0]
 *			n	MAIR
 *   DEVICE_nGnRnE	000	00000000
 *   NORMAL_NC		001	01000100
 */
.equ MT_DEVICE_nGnRnE,                      0x0  // Device nGnRnE memory
.equ MT_NORMAL_NC,                          0x1  // Normal Non-Cacheable memory
.equ MT_NORMAL,                             0x2  // Normal Cacheable memory

.equ MT_DEVICE_nGnRnE_FLAGS,                0x00 // Device nGnRnE attributes
.equ MT_NORMAL_NC_FLAGS,               0x44 // Normal Non-Cacheable attributes
.equ MT_NORMAL_FLAGS,                  0xff // Normal Cacheable attributes

.equ MAIR_VALUE,                            (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC)) | (MT_NORMAL_FLAGS << (8 * MT_NORMAL)) | 


.equ MMU_FLAGS,	 		                    (MM_TYPE_BLOCK | (MT_NORMAL_NC_EXEC << 2) | MM_ACCESS)	
.equ MMU_DEVICE_FLAGS,		                (MM_TYPE_BLOCK | (MT_DEVICE_nGnRnE << 2) | MM_ACCESS)	
.equ MMU_PTE_FLAGS,			                (MM_TYPE_PAGE | (MT_NORMAL_NC << 2) | MM_ACCESS | MM_ACCESS_PERMISSION)	

.equ TCR_T0SZ,			                    (64 - 48) 
.equ TCR_T1SZ,			                    ((64 - 48) << 16)
.equ TCR_TG0_4K,			                (0 << 14)
.equ TCR_TG1_4K,			                (2 << 30)
.equ TCR_VALUE,			                    (TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)
.equ SCTLR_MMU_ENABLED,                     (1 << 0)
.equ SCTLR_MMU_ENABLED_WITH_CACHE,          ((1 << 0) | (1 << 12) | (1 << 2))
