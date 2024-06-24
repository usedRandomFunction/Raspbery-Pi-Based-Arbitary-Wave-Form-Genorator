#include "lib/clocks.h"

#include "io/propertyTags.h"
#include "lib/alloc.h"

static uint32_t s_get_clock_rate_of_type(uint32_t clock_id, uint32_t type);

uint32_t get_clock_rate_messured(uint32_t clock_id)
{
    return s_get_clock_rate_of_type(clock_id, PROPERTY_TAG_ID_GET_CLOCK_RATE_MEASURED);
}

uint32_t get_maximum_clock_rate(uint32_t clock_id)
{
    return s_get_clock_rate_of_type(clock_id, PROPERTY_TAG_ID_GET_MAX_CLOCK_RATE);
}

uint32_t get_minimum_clock_rate(uint32_t clock_id)
{
    return s_get_clock_rate_of_type(clock_id, PROPERTY_TAG_ID_GET_MIN_CLOCK_RATE);
}

uint32_t get_clock_rate(uint32_t clock_id)
{
    return s_get_clock_rate_of_type(clock_id, PROPERTY_TAG_ID_GET_CLOCK_RATE);
}

uint32_t set_clock_rate(uint32_t clock_id, uint32_t rate)
{
    return set_clock_rate_given_alloc_functions(clock_id, rate, aligned_alloc_noncachable, free_noncachable_memory);
}

uint32_t set_clock_rate_given_alloc_functions(uint32_t clock_id, uint32_t rate, MALLOC_ALIGNED_PTR _malloc, FREE_PTR _free)
{
    property_tag_set_clock_rate set_clock_rate_tag;
    set_clock_rate_tag.header.tagID = PROPERTY_TAG_ID_SET_CLOCK_RATE;
    set_clock_rate_tag.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    set_clock_rate_tag.header.buffersize = PROPERTY_TAG_SET_CLOCK_RATE_REQUEST_RESPONSE_SIZE;
    set_clock_rate_tag.rate = rate;
    set_clock_rate_tag.skip_setting_turbo = 0;
    set_clock_rate_tag.clock_id = clock_id;

    property_tag_set_clock_rate_responce* set_clock_rate_tag_responce = (property_tag_set_clock_rate_responce*)get_property_tag((property_tag*)&set_clock_rate_tag, _malloc, _free);

    if (set_clock_rate_tag_responce == NULL)
        return 0;

    rate = set_clock_rate_tag_responce->rate;

    _free(set_clock_rate_tag_responce);

    return rate;
}

#pragma GCC push_options // For some reason gcc -O2 does not like this function
#pragma GCC optimize ("O1")
static uint32_t s_get_clock_rate_of_type(uint32_t clock_id, uint32_t type)
{
    property_tag_get_clock_rate get_clock_rate_tag;
    get_clock_rate_tag.header.tagID = type;
    get_clock_rate_tag.header.request = PROPERTY_TAG_PROCESS_REQUEST;
    get_clock_rate_tag.header.buffersize = PROPERTY_TAG_GET_CLOCK_RATE_REQUEST_RESPONSE_SIZE;
    get_clock_rate_tag.clock_id = clock_id;
    property_tag_get_clock_rate_responce* get_clock_rate_tag_responce = (property_tag_get_clock_rate_responce*)get_property_tag((property_tag*)&get_clock_rate_tag, aligned_alloc_noncachable, free_noncachable_memory);

    if (get_clock_rate_tag_responce == NULL)
        return 0;

    uint32_t rate = get_clock_rate_tag_responce->rate;

    free_noncachable_memory(get_clock_rate_tag_responce);

    return rate;
}
#pragma GCC pop_options
