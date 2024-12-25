#include "common/program_managment.h"
#include "abstracted_outputs/raw.h"
#include "common/basic_io.h"
#include "common/output.h"
#include "common/keypad.h"
#include "common/math.h"


uint64_t* output_buffer = (void*)0xC0000000;

#define BUFFER_SIZE (16 * 1024 * 1024) // 16 MiB

int main()
{
    if (vmemmap(output_buffer, BUFFER_SIZE, VMEMMAP_WRITABILITY) < BUFFER_SIZE)
    {
        printf("Failed to allocate buffer!\n");
        return -1;
    }

    if (dac_resolution(8) != 8)
    {
        printf("Failed to set DAC resolution!\n");
        return -2;
    }

    const int sample_rate = dac_get_sample_rate(8);

    if (sample_rate == 0 || sample_rate == 0xFFFFFFFF)
    {
        printf("Bad sample rate.\n");
        return -3;
    }

    const int freqency = 1000;
    const int required_samples = sample_rate / freqency;
    const double conversion_factor = M_PI_M_2 / required_samples;

    for (int i = 0; i < required_samples; i++)
    {
        float voltage = 2.5f + sin(conversion_factor * i);
        voltage += + sin(conversion_factor * i * 2);;

        int value = voltage_to_dac_value(voltage, 8);

        write_dac_buffer_entry(output_buffer, value, i, 0, 8, 2, 18);
    }

    printf("Remaping PRG_EXIT.\n");
    capture_prg_exit(dac_output_end);
    printf("Dissabling keypad.\n");
    keypad_polling(0);

    printf("Output live.\n");
    dac_output_start(output_buffer, required_samples, DAC_OUTPUT_FLAGS_CH1_ENABLED | DAC_OUTPUT_FLAGS_FRAME_SIZE_2_BITS);

    printf("Done\n");
    capture_prg_exit(NULL);
    keypad_polling(-2);

    return 0;
}