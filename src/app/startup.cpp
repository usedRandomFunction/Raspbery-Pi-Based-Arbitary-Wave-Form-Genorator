#include "app/startup.h"

#include "io/propertyTags.h"
#include "lib/clocks.h"
#include "lib/alloc.h"
#include "io/uart.h"


static void s_PrintMaxiumClockSpeedAndSet(uint32_t clock_id, const char* name, float targetSpeed)
{
    uint32_t maximum_rate = get_maximum_clock_rate(clock_id, aligned_alloc, free);

    if (maximum_rate == 0)
    {
        uart_puts("Failed to get "); uart_puts(name); 
        uart_puts(" maxium clock speed.\n");
    }

    uart_puts(name); uart_puts(" maxium clock speed: ");
    uart_putui(maximum_rate / 1000000);
    uart_puts(" MHz\n");

    uint32_t returnedRate = set_clock_rate(clock_id, (uint32_t)(maximum_rate * targetSpeed), aligned_alloc, free);

    uart_puts("Set ");uart_puts(name); uart_puts(" clock speed to ");
    uart_putui(returnedRate / 1000000);
    uart_puts(" MHz\n");

    returnedRate = get_clock_rate_messured(clock_id, aligned_alloc, free);

    uart_puts("Messured ");uart_puts(name); uart_puts(" clock speed to ");
    uart_putui(returnedRate / 1000000);
    uart_puts(" MHz\n");
}

void SetupSystemClocks(float targetSpeed)
{
    s_PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", targetSpeed);
    s_PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", targetSpeed);
}

void PrintSystemSpecs()
{
    property_tag_get_arm_memory arm_memory_tag;
    arm_memory_tag.tagID = PROPERTY_TAG_ID_GET_ARM_MEMORY;
    arm_memory_tag.request = PROPERTY_TAG_PROCESS_REQUEST;
    arm_memory_tag.buffersize = PROPERTY_TAG_GET_ARM_MEMORY_REQUEST_RESPONSE_SIZE;
    
    auto* arm_memory_tag_responce = (property_tag_get_arm_memory_responce*)get_property_tag((property_tag*)&arm_memory_tag, aligned_alloc, free);
    if (arm_memory_tag_responce != NULL)
    {
        uart_puts("Arm base address: ");
        uart_put_number_as_hex(arm_memory_tag_responce->base_address);
        uart_puts("\nArm memory size: ");
        uart_put_number_as_hex(arm_memory_tag_responce->size);
        uart_putc('\n');
        free(arm_memory_tag_responce);
    }
    else
        uart_puts("\nFailed to get arm memory infomation!\n");


    property_tag_get_temperature get_temperature;
    get_temperature.header.buffersize = PROPERTY_TAG_GET_MAX_TEMPERATURE_REQUEST_RESPONSE_SIZE;
    get_temperature.header.tagID = PROPERTY_TAG_ID_GET_MAX_TEMPERATURE;
    get_temperature.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_temperature.temperature_id = 0;

    auto* get_temperature_responce = (property_tag_get_temperature_responce*)get_property_tag((property_tag*)&get_temperature, aligned_alloc, free);
    if (get_temperature_responce != NULL)
    {
        uart_puts("Max safe SOC temp: ");
        uart_putf(get_temperature_responce->value / 1000.0f, 2);
        uart_puts(" C (");
        uart_putui(get_temperature_responce->value);
        uart_puts(")\n");

        free(get_temperature_responce);
    }
    else
        uart_puts("\nFailed to get maximum SOC temperature\n");

    get_temperature.header.buffersize = PROPERTY_TAG_GET_TEMPERATURE_REQUEST_RESPONSE_SIZE;
    get_temperature.header.tagID = PROPERTY_TAG_ID_GET_TEMPERATURE;
    get_temperature.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_temperature.temperature_id = 0;

    get_temperature_responce = (property_tag_get_temperature_responce*)get_property_tag((property_tag*)&get_temperature, aligned_alloc, free);
    if (get_temperature_responce != NULL)
    {
        uart_puts("SOC temp: ");
        uart_putf(get_temperature_responce->value / 1000.0f, 2);
        uart_puts(" C (");
        uart_putui(get_temperature_responce->value);
        uart_puts(")\n");

        free(get_temperature_responce);
    }
    else
        uart_puts("\nFailed to get soc temperature\n");
}