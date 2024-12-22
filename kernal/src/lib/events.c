#include "lib/events.h"

#include "lib/exceptions.h"
#include "lib/memory.h"
#include "io/printf.h"


static STANDERED_EVENT_HANDLER s_interupt_end_hanlders[INTERUPT_END_EVENT_MAX_HANDLERS];


void initialize_event_handler()
{
    memclr(s_interupt_end_hanlders, sizeof(s_interupt_end_hanlders));
}


void event_handler_add_interupt_end(STANDERED_EVENT_HANDLER handler)
{
    for (int i = 0; i < INTERUPT_END_EVENT_MAX_HANDLERS; i++)
    {
        if (s_interupt_end_hanlders[i] != NULL)
            continue;   // Skip non-null entrys

        s_interupt_end_hanlders[i] = handler;

        return;
    }

    printf("Failed to add handler for event \"%s\"\n", "INTERUPT_END");
    kernel_panic();
}

void event_handler_on_interupt_end()
{
    for (int i = 0; i < INTERUPT_END_EVENT_MAX_HANDLERS; i++)
    {
        if (s_interupt_end_hanlders[i] == NULL)
            continue;   // Skip null entrys

        STANDERED_EVENT_HANDLER handler = s_interupt_end_hanlders[i];
        s_interupt_end_hanlders[i] = NULL;

        handler();
    }
}