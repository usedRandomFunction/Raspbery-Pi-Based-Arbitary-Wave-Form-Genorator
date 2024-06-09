#include "io/propertyTags.h"
#include "app/startup.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "io/gpio.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#if defined(__cplusplus)
extern "C" /* Use C linkage for main. */
#endif
int main()
{
    SetupSystemClocks(1.2f);

    gpio_function_select(17, GPFSEL_Output); // Clock
    gpio_function_select(18, GPFSEL_Output); // Data
    gpio_function_select(27, GPFSEL_Output); // Latch

    auto* set_address = get_mmio_pointer(GPSET0);
    auto* clr_address = get_mmio_pointer(GPCLR0);

    constexpr uint32_t clock_mask = 1 << 17;
    constexpr uint32_t data_mask = 1 << 18;
    constexpr uint32_t both_mask = clock_mask | data_mask;
    constexpr uint32_t mask_all = UINT32_MAX;

    while (1)
	{
        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(0));
        *set_address = data_mask & (0);
        *set_address = clock_mask;

        *clr_address = clock_mask | (data_mask & ~(mask_all));
        *set_address = data_mask & (mask_all);
        *set_address = clock_mask;

        gpio_write(27, 1);
        gpio_write(27, 0);
	}

    return 0;
}