// heavyly based off https://github.com/bztsrc/raspi3-tutorial/blob/master/0B_readsector/sd.c

#include "io/sd.h"

#include "io/memoryMappedIO.h"
#include "io/propertyTags.h"
#include "lib/timing.h"
#include "lib/clocks.h"
#include "io/putchar.h"
#include "lib/memory.h"
#include "io/printf.h"
#include "io/gpio.h"

#include <stdbool.h>

// command flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// commands
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_WRITE_SINGLE    0x18220000
#define CMD_WRITE_MULTI     0x19220022
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_READ_AVAILABLE   0x00000800
#define SR_WRITE_AVAILABLE  0x00000400
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_WRITE_RDY       0x00000010
#define INT_DATA_DONE       0x00000002
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
// added by my driver
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

static uint32_t s_sd_hv;
static uint32_t s_sd_scr[2];
static uint32_t s_sd_ocr;
static uint32_t s_sd_rca; 
static uint32_t s_sd_err;

// Sets the frequency of the SD clock
// @param frequency the target frequency in HZ
// @return SD_OK if succcess
static int s_sd_set_freqency(uint32_t frequency);

// Sends a command to the SD
// @param code Command code
// @param arg Argument
// @return depenuart_putsds on command
static int s_sd_command(uint32_t code, uint32_t arg);

// Waits for data or command ready
// @param mask Bit mask for EMMC_STATUS
// @return SD_OK if ok
static int s_sd_status(uint32_t mask);

// Waits for interupt
// @param mask Mask for the interup
// @return 0 if ok
static int s_sd_int(uint32_t mask);

int initialize_sd()
{
    int return_value = SD_OK;
    printf("Initializing sd card\n");

    gpio_function_select(47, GPFSEL_Input);

    mmio_write(GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPPUDCLK1, 1 << 15);
    wait_cycles(150);
    mmio_write(GPPUD, 0);
    mmio_write(GPPUDCLK1, 0);

    mmio_write_bitwise_or(GPHEN1, 1 << 15);

    gpio_function_select(48, GPFSEL_Alternate3);
    gpio_function_select(49, GPFSEL_Alternate3);

    mmio_write(GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPPUDCLK1, 1 << 16 | 1 << 17);
    wait_cycles(150);
    mmio_write(GPPUD, 0);
    mmio_write(GPPUDCLK1, 0);

    gpio_function_select(50, GPFSEL_Alternate3);
    gpio_function_select(51, GPFSEL_Alternate3);
    gpio_function_select(52, GPFSEL_Alternate3);
    gpio_function_select(53, GPFSEL_Alternate3);

    mmio_write(GPPUD, 2);
    wait_cycles(150);
    mmio_write(GPPUDCLK1, (1<<18) | (1<<19) | (1<<20) | (1<<21));
    wait_cycles(150);
    mmio_write(GPPUD, 0);
    mmio_write(GPPUDCLK1, 0);

    s_sd_hv = (mmio_read(EMMC_SLOTISR_VER) & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;

    printf("EMMC: GPIO set up, sd_hc = %x\nEMMC: Reseting SD\n", s_sd_hv);

    mmio_write(EMMC_CONTROL0, 0);
    mmio_write_bitwise_or(EMMC_CONTROL1, C1_SRST_HC);
    bool successs = false;

    for (int i = 0; i < 100000; i++)
    {
        if (!(mmio_read(EMMC_CONTROL1) & C1_SRST_HC))
        {
            successs = true;
            break;
        }
        delay_microseconds(10);
    }

    if (successs == false)
    {
        printf("EMMC: ERRs_sd_statusOR: failed to reset EMMC\n");
        return SD_ERR;
    }
    printf("EMMC: Reset OK\n");

    mmio_write_bitwise_or(EMMC_CONTROL1, C1_CLK_INTLEN | C1_TOUNIT_MAX);
    delay_microseconds(10);

    return_value = s_sd_set_freqency(400000);

    if (return_value != SD_OK)
        return return_value;

    mmio_write(EMMC_INT_EN, 0xFFFFFFFF);
    mmio_write(EMMC_INT_MASK, 0xFFFFFFFF);

    s_sd_scr[1] = 0;
    s_sd_scr[0] = 0;
    s_sd_ocr = 0;
    s_sd_rca = 0;
    s_sd_err = 0;

    s_sd_command(CMD_GO_IDLE, 0);
    if(s_sd_err) 
        return s_sd_err;

    s_sd_command(CMD_SEND_IF_COND, 0x000001AA);
    if(s_sd_err) 
        return s_sd_err;


    uint32_t r = 0;

    successs = false;

    for (int i = 0; i < 6; i++)
    {
        wait_cycles(2000);
        r = s_sd_command(CMD_SEND_OP_COND, ACMD41_ARG_HC);

        #ifdef SD_VERBOSE_LOGGING
        printf("EMMC: CMD_SEND_OP_COND returned ");

        if(r & ACMD41_CMD_COMPLETE)
            printf("COMPLETE ");

        if(r & ACMD41_VOLTAGE)
            printf("VOLTAGE ");

        if(r & ACMD41_CMD_CCS)
            printf("CCS ");
        
        printf("%x\n", r);
        #endif

        if(s_sd_err != SD_TIMEOUT && s_sd_err != SD_OK ) {
            printf("EMMC: ERROR: ACMD41 returned error\n");
            return s_sd_err;
        }

        if (r & ACMD41_CMD_COMPLETE)
        {
            successs = true;
            break;
        }
    }

    if(!successs)
    {
        printf("EMMC: ERROR: ACMD41 timeout\n");
        return SD_TIMEOUT;
    }

    if(!(r & ACMD41_VOLTAGE)) 
    {
        printf("EMMC: ERROR: ACMD41 voltage\n");
        return SD_ERR;
    }

    int32_t ccs = 0;

    if (r & ACMD41_CMD_CCS)
        ccs = SCR_SUPP_CCS;

    s_sd_command(CMD_ALL_SEND_CID, 0);

    s_sd_rca = s_sd_command(CMD_SEND_REL_ADDR, 0);

    #ifdef SD_VERBOSE_LOGGING
    printf("EMMC: CMD_SEND_REL_ADDR returned %x\n", s_sd_rca);
    #endif

    if (s_sd_err)
        return s_sd_err;

    if((return_value = s_sd_set_freqency(25000000))) 
        return return_value;

    s_sd_command(CMD_CARD_SELECT, s_sd_rca);
    if (s_sd_err)
        return s_sd_err;

    if(s_sd_status(SR_DAT_INHIBIT)) 
        return SD_TIMEOUT;
    mmio_write(EMMC_BLKSIZECNT, (1<<16) | 8);
    s_sd_command(CMD_SEND_SCR,0);
    if(s_sd_err) 
        return s_sd_err;

    if(s_sd_int(INT_READ_RDY)) 
        return SD_TIMEOUT;

    r = 0;

    for (int i = 0; i < 100000; i++)
    {
        if ((mmio_read(EMMC_STATUS) & SR_READ_AVAILABLE))
        {
            s_sd_scr[r++] = mmio_read(EMMC_DATA);
            if (r >= 2)
                break;
        }
        else
            delay_microseconds(1);
    }

    if (r != 2)
        return SD_TIMEOUT;

    if(s_sd_scr[0] & SCR_SD_BUS_WIDTH_4) 
    {
        s_sd_command(CMD_SET_BUS_WIDTH, s_sd_rca | 2);
        if(s_sd_err) 
            return s_sd_err;

        mmio_write_bitwise_or(EMMC_CONTROL0, C0_HCTL_DWITDH);
    }
    // add software flag
    #ifdef SD_VERBOSE_LOGGING
    printf("EMMC: supports ");
    if(s_sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        printf("SET_BLKCNT ");

    if(ccs)
        printf("CCS ");

    putchar('\n');
    #endif
    s_sd_scr[0]&=~SCR_SUPP_CCS;
    s_sd_scr[0]|=ccs;

    printf("SD card Initialized\n");

    return return_value;
}


int s_sd_set_freqency(uint32_t frequency)
{
    bool successs = false;
    for (int i = 0; i < 100000; i++)
    {
        if (!(mmio_read(EMMC_STATUS) & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)))
        {
            successs = true;
            break;
        }

        delay_microseconds(1);
    }
    
    if (successs == false)
    {
        printf("EMMC: ERROR: timeout waiting for inhibit flag\n");
        return SD_ERR;
    }

    mmio_write_bitwise_and(EMMC_CONTROL1, ~C1_CLK_EN);
    delay_microseconds(10);

    uint32_t EMMC_base_frequency = get_clock_rate_messured(PROPERTY_TAG_CLOCK_ID_EMMC);

    uint32_t divisor = 0;
    uint32_t shift = 32;
    uint32_t c = EMMC_base_frequency / frequency;
    uint32_t x = c - 1;

    if(!x) 
        shift = 0; 
    else {
        if(!(x & 0xffff0000u)) { x <<= 16; shift -= 16; }
        if(!(x & 0xff000000u)) { x <<= 8;  shift -= 8; }
        if(!(x & 0xf0000000u)) { x <<= 4;  shift -= 4; }
        if(!(x & 0xc0000000u)) { x <<= 2;  shift -= 2; }
        if(!(x & 0x80000000u)) { x <<= 1;  shift -= 1; }
        if(shift > 0) shift--;
        if(shift > 7) shift=7;
    }

    if (s_sd_hv > HOST_SPEC_V2)
        divisor = c;
    else
        divisor = 1 << shift;

    if(divisor<=2) 
    {
        divisor=2;
        shift=0;
    }

    #ifdef SD_VERBOSE_LOGGING
    printf("EMMC: sd_clk divisor: %x, shift: %x\n", divisor, shift);
    #endif

    uint32_t h = 0;

    if(s_sd_hv > HOST_SPEC_V2) 
        h = (divisor & 0x300) >> 2;

    divisor = (((divisor & 0x0ff) << 8) | h);
    mmio_write_bitwise_and(EMMC_CONTROL1, 0xffff003f);
    mmio_write_bitwise_or(EMMC_CONTROL1, divisor);
    delay_microseconds(10);

    mmio_write_bitwise_or(EMMC_CONTROL1, C1_CLK_EN);
    delay_milliseconds(10);
    
    successs = false;
    for (int i = 0; i < 10000; i++)
    {
        if (mmio_read(EMMC_CONTROL1) & C1_CLK_STABLE)
        {
            successs = true;
            break;
        }

        delay_microseconds(10);
    }
    
    if (successs == false)
    {
        printf("EMMC: ERROR: failed to get stable clock\n");
        return SD_ERR;
    }

    return SD_OK;
}

static int s_sd_command(uint32_t code, uint32_t arg)
{
    int _return = 0;
    s_sd_err = SD_OK;

    if (code & CMD_NEED_APP) 
    {
        _return = s_sd_command(CMD_APP_CMD | (s_sd_rca ? CMD_RSPNS_48 : 0), s_sd_rca);

        if(s_sd_rca && !_return) 
        { 
            printf("EMMC: ERROR: failed to send SD APP command\n"); 
            s_sd_err = SD_ERR;
            return 0;
        }

        code &= ~CMD_NEED_APP;
    }

    if(s_sd_status(SR_CMD_INHIBIT)) 
    { 
        printf("EMMC: ERROR: busy\n"); 
        s_sd_err = SD_TIMEOUT;
        return 0;
    }

    #ifdef SD_VERBOSE_LOGGING
    printf("EMMC: Sending command %x arg %x\n", code, arg);
    #endif
    delay_microseconds(5000);

    mmio_write(EMMC_INTERRUPT, mmio_read(EMMC_INTERRUPT)); // Im not sure why but we need to do this
    mmio_write(EMMC_ARG1, arg);
    mmio_write(EMMC_CMDTM, code);

    if(code == CMD_SEND_OP_COND) 
    {
        delay_microseconds(1000); 
    } else if(code == CMD_SEND_IF_COND || code == CMD_APP_CMD) 
    {
        delay_microseconds(100);
    }

    if((_return = s_sd_int(INT_CMD_DONE))) 
    {
        printf("EMMC: ERROR: failed to send command\n");
        s_sd_err = _return;
        return 0;
    }

    _return = mmio_read(EMMC_RESP0);

    if (code == CMD_GO_IDLE || code == CMD_APP_CMD) 
    {
        return 0;
    } else if(code == (CMD_APP_CMD | CMD_RSPNS_48))
    {
        return _return & SR_APP_CMD;
    } else if(code==CMD_SEND_OP_COND) 
    {
        return _return;
    } else if(code==CMD_SEND_IF_COND) 
    {
        return _return == arg ? SD_OK : SD_ERR;
    } else if(code==CMD_ALL_SEND_CID) 
    {
        _return |= mmio_read(EMMC_RESP3); 
        _return |= mmio_read(EMMC_RESP2); 
        _return |= mmio_read(EMMC_RESP1); 
        return _return; 
    } else if(code==CMD_SEND_REL_ADDR) {
        s_sd_err=(((_return & 0x1fff)) | ((_return & 0x2000) << 6) | ((_return & 0x4000) <<8 ) | ((_return & 0x8000) << 8)) & CMD_ERRORS_MASK;
        return _return & CMD_RCA_MASK;
    }
    return _return & CMD_ERRORS_MASK;
}

int s_sd_status(uint32_t mask)
{   
    bool successs = false;

    for (int i = 0; i < 100000; i++)
    {
        if (!(mmio_read(EMMC_STATUS) & mask))
        {
            successs = true;
            break;
        }
        delay_microseconds(1);
    }

    if (successs == false || (mmio_read(EMMC_INTERRUPT) & INT_ERROR_MASK))
        return SD_ERR;

    return SD_OK;
}

int s_sd_int(uint32_t mask)
{   
    uint32_t return_value = 0;
    uint32_t bitmask = mask | INT_ERROR_MASK;

    bool successs = false;

    for (int i = 0; i < 1000000; i++)
    {   
        return_value = mmio_read(EMMC_INTERRUPT);
        if (return_value & bitmask)
        {
            successs = true;
            break;
        }
        delay_microseconds(1);
    }

    if (successs == false || (return_value & INT_CMD_TIMEOUT) || (return_value & INT_DATA_TIMEOUT))
    {
        mmio_write(EMMC_INTERRUPT, return_value);
        return SD_TIMEOUT;
    }

    if(return_value & INT_ERROR_MASK) 
    {
        mmio_write(EMMC_INTERRUPT, return_value);
        return SD_ERR;
    }
    
    
    mmio_write(EMMC_INTERRUPT, mask);

    return 0;
}

int sd_readblock(uint32_t lba, void* buf, uint32_t num)
{
    uint32_t return_value = 0;
    uint32_t count = 0;

    if (num == 0)
        return 0;
    
    #ifdef SD_VERBOSE_LOGGING
    printf("Reading block 0x%x (lba), n = %x from SD\n", lba, num);
    #endif
    
    if(s_sd_status(SR_DAT_INHIBIT)) 
    {
        s_sd_err = SD_TIMEOUT; 
        return 0;
    }

    uint32_t* buffer = (uint32_t*)buf;

    if(s_sd_scr[0] & SCR_SUPP_CCS) 
    {
        if(num > 1 && (s_sd_scr[0] & SCR_SUPP_SET_BLKCNT)) 
        {
            s_sd_command(CMD_SET_BLOCKCNT,num);
            if(s_sd_err) 
                return 0;
        }
        mmio_write(EMMC_BLKSIZECNT, (num << 16) | 512);
        s_sd_command(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI, lba);
        if(s_sd_err) 
            return 0;
    } 
    else 
    {
        mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    }

    while (count < num) 
    {
        if(!(s_sd_scr[0] & SCR_SUPP_CCS)) 
        {
            s_sd_command(CMD_READ_SINGLE, (lba + count) * 512);

            if(s_sd_err) 
              return 0;
        }

        if((return_value = s_sd_int(INT_READ_RDY)))
        {
            printf("ERROR: Timeout waiting for ready to read\n");
            s_sd_err = return_value;
            return 0;
        }

        for(int i = 0; i < 128; i++) 
            buffer[i] = mmio_read(EMMC_DATA);
        count++; 
        buffer+=128;
    }

    if( num > 1 && !(s_sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (s_sd_scr[0] & SCR_SUPP_CCS)) 
        s_sd_command(CMD_STOP_TRANS,0);

    return s_sd_err != SD_OK || count != num ? 0 : num * 512;
}

int sd_writeblock(uint32_t lba, const void* buf, uint32_t num)
{
    if (num == 0)
        return 0;

        
    #ifdef SD_VERBOSE_LOGGING
    printf("Writing block 0x%x (lba), n = %x from SD\n", lba, num);
    #endif
    
    if(s_sd_status(SR_DAT_INHIBIT | SR_WRITE_AVAILABLE)) 
    {
        s_sd_err = SD_TIMEOUT; 
        return 0;
    }

    unsigned int* bufffer = (unsigned int*)buf;
    if(s_sd_scr[0] & SCR_SUPP_CCS) 
    {
        if(num > 1 && (s_sd_scr[0] & SCR_SUPP_SET_BLKCNT)) 
        {
            s_sd_command(CMD_SET_BLOCKCNT, num);
            if(s_sd_err) 
                return 0;
        }
        mmio_write(EMMC_BLKSIZECNT, (num << 16) | 512);
        s_sd_command(num == 1 ? CMD_WRITE_SINGLE : CMD_WRITE_MULTI, lba);
        if(s_sd_err) 
            return 0;
    } 
    else 
    {
        mmio_write(EMMC_BLKSIZECNT, (1 << 16) | 512);
    }

    uint32_t count = 0;
    int r;

    while(count < num) 
    {
        if(!(s_sd_scr[0] & SCR_SUPP_CCS)) 
        {
            s_sd_command(CMD_WRITE_SINGLE, (lba + count) * 512);
            if(s_sd_err) 
                return 0;
        }
        if((r = s_sd_int(INT_WRITE_RDY)))
        {
            printf("ERROR: Timeout waiting for ready to write\n");
            s_sd_err = r;
            return 0;
        }
        for(int d = 0; d < 128; d++) 
            mmio_write(EMMC_DATA, bufffer[d]);
        bufffer+=128;
        count++; 
    }

    if((r = s_sd_int(INT_DATA_DONE)))
    {
        printf("ERROR: Timeout waiting for data done\n");
        s_sd_err = r;
        return 0;
    }

    if( num > 1 && !(s_sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (s_sd_scr[0] & SCR_SUPP_CCS)) 
        s_sd_command(CMD_STOP_TRANS,0);

    return s_sd_err!=SD_OK || count != num ? 0 : num * 512;
}


// The code above this is just a neater rewrite of a example i've found
// From hear fully custom

int sd_write_section(uint32_t lba, void* buf, uint32_t offset, uint32_t num, uint32_t section_size, void* section)
{   
    if (offset + num > (section_size * 512))
        return 0; // If we are trying to write to memory out side the section just dont try

    bool section_memory_was_provided = section != NULL;

    if (section == NULL)
        section = malloc(section_size * 512);

    if (section == NULL) // Failed to allocate
        return 0;

    if (sd_readblock(lba, section, section_size) != section_size * 512)
    {
        if (!section_memory_was_provided)
            free(section);

        return 0;
    }

    memcpy(void_ptr_offset_bytes(section, offset), buf, num);

    if (sd_writeblock(lba, section, section_size) != section_size * 512)
    {
        if (!section_memory_was_provided)
            free(section);

        return 0;
    }

    return 1;
}