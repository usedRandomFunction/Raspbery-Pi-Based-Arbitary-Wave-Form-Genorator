#include "app/startup.h"

#include "io/propertyTags.h"
#include "lib/clocks.h"
#include "lib/alloc.h"
#include "io/printf.h"


void PrintMaxiumClockSpeedAndSet(uint32_t clock_id, const char* name, float targetSpeed)
{
    uint32_t maximum_rate = get_maximum_clock_rate(clock_id);

    if (maximum_rate == 0)
    {
        printf("Failed to get %s maxium clock speed\n", name);
    }

    printf("%s maxium clock speed: %d MHz\n", name, maximum_rate / 1000000);

    uint32_t returnedRate = set_clock_rate(clock_id, (uint32_t)(maximum_rate * targetSpeed));

    printf("Set %s clock speed to %d MHz\n", name, returnedRate / 1000000);

    returnedRate = get_clock_rate_messured(clock_id);

    printf("Messured %s clock speed to %d MHz\n", name, returnedRate / 1000000);
}

void SetupSystemClocks(float targetSpeed)
{
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_ARM, "Arm", targetSpeed);
    PrintMaxiumClockSpeedAndSet(PROPERTY_TAG_CLOCK_ID_CORE, "Core", targetSpeed);
}

void PrintSystemSpecs()
{
    property_tag_get_arm_memory arm_memory_tag;
    arm_memory_tag.header.tagID = PROPERTY_TAG_ID_GET_ARM_MEMORY;
    arm_memory_tag.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    arm_memory_tag.header.buffersize = PROPERTY_TAG_GET_ARM_MEMORY_REQUEST_RESPONSE_SIZE;
    
    property_tag_get_arm_memory_responce* arm_memory_tag_responce = (property_tag_get_arm_memory_responce*)get_property_tag((property_tag*)&arm_memory_tag, aligned_alloc, free);
    if (arm_memory_tag_responce != NULL)
    {
        printf("Arm base address: %x\nArm memory size: %x\n",
            arm_memory_tag_responce->base_address,
            arm_memory_tag_responce->size);
        free(arm_memory_tag_responce);
    }
    else
        printf("\nFailed to get arm memory infomation!\n");


    property_tag_get_temperature get_temperature;
    get_temperature.header.buffersize = PROPERTY_TAG_GET_MAX_TEMPERATURE_REQUEST_RESPONSE_SIZE;
    get_temperature.header.tagID = PROPERTY_TAG_ID_GET_MAX_TEMPERATURE;
    get_temperature.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_temperature.temperature_id = 0;

    property_tag_get_temperature_responce* get_temperature_responce = (property_tag_get_temperature_responce*)get_property_tag((property_tag*)&get_temperature, aligned_alloc, free);
    if (get_temperature_responce != NULL)
    {
        printf("Max safe SOC temp: %d C (%d)\n",
           get_temperature_responce->value / 1000,
           get_temperature_responce->value);

        free(get_temperature_responce);
    }
    else
        printf("\nFailed to get maximum SOC temperature\n");

    get_temperature.header.buffersize = PROPERTY_TAG_GET_TEMPERATURE_REQUEST_RESPONSE_SIZE;
    get_temperature.header.tagID = PROPERTY_TAG_ID_GET_TEMPERATURE;
    get_temperature.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_temperature.temperature_id = 0;

    get_temperature_responce = (property_tag_get_temperature_responce*)get_property_tag((property_tag*)&get_temperature, aligned_alloc, free);
    if (get_temperature_responce != NULL)
    {
        printf("SOC temp: %d C (%d)\n",
           get_temperature_responce->value / 1000,
           get_temperature_responce->value);

        free(get_temperature_responce);
    }
    else
        printf("\nFailed to get soc temperature\n");
}