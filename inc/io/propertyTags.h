#ifndef PROPERYTAGS_H
#define PROPERYTAGS_H

#include "lib/memory.h"

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

#pragma region tags

struct property_tag
{
    // the ID of the tag
    uint32_t tagID;

    // the size of the tag's contents not including the header
    uint32_t buffersize;

    // should be PROPERTY_TAG_PROCESS_REQUEST
    uint32_t request;
}__attribute__((packed));

typedef struct property_tag property_tag;

struct property_tag_get_temperature
{
    property_tag header;

    // Should be zero
    uint32_t temperature_id;
}__attribute__((packed));

typedef struct property_tag_get_temperature property_tag_get_temperature;
typedef struct property_tag_get_temperature property_tag_get_max_temperature;

struct property_tag_get_temperature_responce
{
    property_tag header;

    // Should be zero
    uint32_t temperature_id;

    // temperature of the SoC in thousandths of a degree C
    uint32_t value;
}__attribute__((packed));

typedef struct property_tag_get_temperature_responce property_tag_get_temperature_responce;
typedef struct property_tag_get_temperature_responce property_tag_get_max_temperature_responce;

struct property_tag_get_clock_rate
{
    property_tag header;

    uint32_t clock_id;
}__attribute__((packed));

typedef struct property_tag_get_clock_rate property_tag_get_clock_rate;
typedef struct property_tag_get_clock_rate property_tag_get_max_clock_rate;
typedef struct property_tag_get_clock_rate property_tag_get_clock_rate_measured;

struct property_tag_get_clock_rate_responce
{
    property_tag header;

    uint32_t clock_id;

    // The clock rate in hz
    uint32_t rate;
}__attribute__((packed));

typedef struct property_tag_get_clock_rate_responce property_tag_set_clock_rate_responce;
typedef struct property_tag_get_clock_rate_responce property_tag_get_clock_rate_responce;
typedef struct property_tag_get_clock_rate_responce property_tag_get_max_clock_rate_responce;
typedef struct property_tag_get_clock_rate_responce property_tag_get_clock_rate_measured_responce;

struct property_tag_set_clock_rate
{
    property_tag header;

    uint32_t clock_id;

    // The clock rate in hz
    uint32_t rate;

    // Set this to '1' inorder to have dissable auto voltage and freqnecy settings, or '0' to enable
    uint32_t skip_setting_turbo;
}__attribute__((packed));

typedef struct property_tag_set_clock_rate property_tag_set_clock_rate;

struct property_tag_get_arm_memory_responce
{
    property_tag header;

    uint32_t base_address;

    uint32_t size;
}__attribute__((packed));

typedef struct property_tag property_tag_get_arm_memory;
typedef struct property_tag_get_arm_memory_responce property_tag_get_arm_memory_responce;

#pragma endregion

// Writes the given tag and waits for return
// (Identical to write_property_tags(tag, tag->buffersize + sizeof(property_tag), _malloc, _free))
// @param tag A pointer to the tag to write
// @param _malloc A function pointer to alligned alloc
// @param _free A function pointer to free
inline void write_property_tag(property_tag* tag, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);

// Writes the given tag and returns the return values, returns NULL if failer
// (Identical to get_property_tags(tag, tag->buffersize + sizeof(property_tag), _malloc, _free))
// @param tag A pointer to the tag to write
// @param _malloc A function pointer to alligned alloc
// @param _free A function pointer to free
// @return The returned tag from the VC
// @note the returned ptr is not the given ptr and need to be freed
inline property_tag* get_property_tag(property_tag* tag, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);

// Writes the given tag buffer, note that the buffer does not include the extra info added to the buffer
// @param buffer The buffer with the tags
// @param buffer_size The size in bytes of the tag buffer
// @param _malloc A function pointer to alligned alloc
// @param _free A function pointer to free
void write_property_tags(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);

// Writes the given tag buffer, note that the buffer does not include the extra info added to the buffer
// @param buffer The buffer with the tags
// @param buffer_size The size in bytes of the tag buffer
// @param _malloc A function pointer to alligned alloc
// @param _free A function pointer to free
// @return The returned list of property tages from the VC
// @note the returned ptr is not the given ptr and needs to be freed
property_tag* get_property_tags(property_tag* buffer, uint32_t buffer_size, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free);



inline void write_property_tag(property_tag* tag, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    write_property_tags(tag, tag->buffersize + sizeof(property_tag), _malloc, _free);
}

inline property_tag* get_property_tag(property_tag* tag, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    return get_property_tags(tag, tag->buffersize + sizeof(property_tag), _malloc, _free);
}

enum 
{
    PROPERTY_TAG_PROCESS_REQUEST            = 0x00000000,
    PROPERTY_TAG_REQUEST_SUCCESSFUL         = 0x80000000,
    PROPERTY_TAG_ERROR                      = 0x80000001,
    PROPERTY_TAG_RESPONSE_MASK              = 0x7FFFFFFF,
    PROPERTY_TAG_ID_END			            = 0x00000000,
    PROPERTY_TAG_ID_GET_FIRMWARE_REVISION	= 0x00000001,
    PROPERTY_TAG_ID_SET_CURSOR_INFO		    = 0x00008010,
    PROPERTY_TAG_ID_SET_CURSOR_STATE	    = 0x00008011,
    PROPERTY_TAG_ID_GET_BOARD_MODEL		    = 0x00010001,
    PROPERTY_TAG_ID_GET_BOARD_REVISION	    = 0x00010002,
    PROPERTY_TAG_ID_GET_MAC_ADDRESS		    = 0x00010003,
    PROPERTY_TAG_ID_GET_BOARD_SERIAL	    = 0x00010004,
    PROPERTY_TAG_ID_GET_ARM_MEMORY		    = 0x00010005,
    PROPERTY_TAG_ID_GET_VC_MEMORY		    = 0x00010006,
    PROPERTY_TAG_ID_SET_POWER_STATE		    = 0x00028001,
    PROPERTY_TAG_ID_GET_CLOCK_RATE		    = 0x00030002,
    PROPERTY_TAG_ID_GET_MAX_CLOCK_RATE	    = 0x00030004,
    PROPERTY_TAG_ID_GET_TEMPERATURE		    = 0x00030006,
    PROPERTY_TAG_ID_GET_MIN_CLOCK_RATE	    = 0x00030007,
    PROPERTY_TAG_ID_GET_TURBO		        = 0x00030009,
    PROPERTY_TAG_ID_GET_MAX_TEMPERATURE	    = 0x0003000A,
    PROPERTY_TAG_ID_GET_EDID_BLOCK		    = 0x00030020,
    PROPERTY_TAG_ID_GET_THROTTLED		    = 0x00030046,
    PROPERTY_TAG_ID_GET_CLOCK_RATE_MEASURED	= 0x00030047,
    PROPERTY_TAG_ID_NOTIFY_XHCI_RESET	    = 0x00030058,
    PROPERTY_TAG_ID_SET_CLOCK_RATE		    = 0x00038002,
    PROPERTY_TAG_ID_SET_TURBO		        = 0x00038009,
    PROPERTY_TAG_ID_SET_DOMAIN_STATE	    = 0x00038030,
    PROPERTY_TAG_ID_SET_SET_GPIO_STATE	    = 0x00038041,
    PROPERTY_TAG_ID_SET_SDHOST_CLOCK	    = 0x00038042,
    PROPERTY_TAG_ID_ALLOCATE_BUFFER		    = 0x00040001,
    PROPERTY_TAG_ID_GET_DISPLAY_DIMENSIONS	= 0x00040003,
    PROPERTY_TAG_ID_GET_PITCH		        = 0x00040008,
    PROPERTY_TAG_ID_GET_TOUCHBUF		    = 0x0004000F,
    PROPERTY_TAG_ID_GET_GPIO_VIRTBUF	    = 0x00040010,
    PROPERTY_TAG_ID_GET_NUM_DISPLAYS	    = 0x00040013,
    PROPERTY_TAG_ID_SET_PHYS_WIDTH_HEIGHT	= 0x00048003,
    PROPERTY_TAG_ID_SET_VIRT_WIDTH_HEIGHT	= 0x00048004,
    PROPERTY_TAG_ID_SET_DEPTH		        = 0x00048005,
    PROPERTY_TAG_ID_SET_VIRTUAL_OFFSET	    = 0x00048009,
    PROPERTY_TAG_ID_SET_PALETTE		        = 0x0004800B,
    PROPERTY_TAG_ID_WAIT_FOR_VSYNC		    = 0x0004800E,
    PROPERTY_TAG_ID_SET_BACKLIGHT		    = 0x0004800F,
    PROPERTY_TAG_ID_SET_DISPLAY_NUM		    = 0x00048013,
    PROPERTY_TAG_ID_SET_TOUCHBUF		    = 0x0004801F,
    PROPERTY_TAG_ID_SET_GPIO_VIRTBUF	    = 0x00048020,
    PROPERTY_TAG_ID_GET_COMMAND_LINE	    = 0x00050001,
    PROPERTY_TAG_ID_GET_DMA_CHANNELS	    = 0x00060001,
};

enum
{
    PROPERTY_TAG_GET_TEMPERATURE_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_GET_MAX_TEMPERATURE_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_GET_CLOCK_RATE_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_GET_CLOCK_RATE_MEASURED_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_GET_MAX_CLOCK_RATE_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_GET_ARM_MEMORY_REQUEST_RESPONSE_SIZE = 0x08,
    PROPERTY_TAG_SET_CLOCK_RATE_REQUEST_RESPONSE_SIZE = 0x0C,
};

enum
{
    PROPERTY_TAG_CLOCK_ID_RESERVED    = 0x0,
    PROPERTY_TAG_CLOCK_ID_EMMC        = 0x1,
    PROPERTY_TAG_CLOCK_ID_UART        = 0x2,
    PROPERTY_TAG_CLOCK_ID_ARM         = 0x3,
    PROPERTY_TAG_CLOCK_ID_CORE        = 0x4,
    PROPERTY_TAG_CLOCK_ID_V3D         = 0x5,
    PROPERTY_TAG_CLOCK_ID_H264        = 0x6,
    PROPERTY_TAG_CLOCK_ID_ISP         = 0x7,
    PROPERTY_TAG_CLOCK_ID_SDRAM       = 0x8,
    PROPERTY_TAG_CLOCK_ID_PIXEL       = 0x9,
    PROPERTY_TAG_CLOCK_ID_PWM         = 0xa,
    PROPERTY_TAG_CLOCK_ID_HEVC        = 0xb,
    PROPERTY_TAG_CLOCK_ID_EMMC2       = 0xc,
    PROPERTY_TAG_CLOCK_ID_M2MC        = 0xd,
    PROPERTY_TAG_CLOCK_ID_PIXEL_BVB   = 0xe,
};
    
#ifdef __cplusplus
}
#endif

#endif