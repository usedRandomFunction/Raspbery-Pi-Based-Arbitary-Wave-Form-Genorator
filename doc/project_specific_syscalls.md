# Project specific syscalls

The project specific syscalls follow the standard as defined in the [v0 abi](./v0_abi.md)

## syscall tables vs SVC argument

## 0x8000 - Hardware outputs (DAC) and settings (buffering / resolution)

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | dac_output_start | void* buffer_start,<br> size_t n, int [flags](#dac_output_start-flags) | int | When called the calling thread will be used by the kernal to <br> output data to the DACs, in the method described in [outputs.md](./outputs.md#transmission-and-storage-in-memory).<br>The parameter `n` stores the number of full values not frames<br>(Note that this is for each channel not all of them added)<br> All other values are controlled with the [flags](#dac_output_start-flags). <br><br> Returns zero if exited with dac_output_end or reached the end of the buffer,<br>and non-zero on failer, the first two bits of return value state what channel is <br>incompitible. If Bit 3 is set then the error is not specific to the channels. 
| 1 | dac_output_end | void | void | When called stops the DAC output on all threads.
| 2 | dac_resolution | int resolution | int | Sets / gets the resolution of all the DACs, supported resolutions are <br> `2`, `8` and `16` bit, if resolution is set to `-1` the return value will simply<br> be the current resolution. Returns the resolution at the end of the operation  <br><br>Note: This function may allow resolution settings not supported by<br>any channels in which case dac_output_start will allways fail.
| 3 | dac_channel_buffering | int channel, int state | int | Enables, dissables or checks the output buffers on the given channel,<br>if `state` = `1` buffering is enabled, if `state` = `0` buffering is dissabled<br>A `state` of `-1` just returns the current state. It will allways return the<br>current / new state or `-1` if a invaild channel is given
| 4 | dac_get_sample_rate | int resolution | int | Returns the sample rate, in hertz, at a given resolution, if a resolution<br>of `-1` is given it will return the sample rate at the current resolution.<br>If the chosen resolution is not supported supported a value of `0` is returned.
| 5 | dac_channel_supports_config | int channel,<br>int resolution,<br>int buffered | int | Returns `0` if the config is supported, bit 0 is set if the buffering state is not,<br> bit 1 is set if the resolution isn't suported. Bit 2 is set if a invaild channel is given<br> For both `resolution` and `buffered` if `-1` is given it will use the current<br>setting.

### dac_output_start flags

| Value | Name | Description |
|-------|------|-------------|
| 0 | DAC_OUTPUT_FLAGS_FRAME_SIZE_2_BITS | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 1 | DAC_OUTPUT_FLAGS_FRAME_SIZE_4_BITS | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 2 | DAC_OUTPUT_FLAGS_FRAME_SIZE_8_BITS | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 0 | DAC_OUTPUT_FLAGS_FRAME_START_CH_1 | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 16 | DAC_OUTPUT_FLAGS_FRAME_START_CH_2 | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 32 | DAC_OUTPUT_FLAGS_FRAME_START_CH_3 | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 48 | DAC_OUTPUT_FLAGS_FRAME_START_CH_4 | Sets details about the output frame [see outputs.md](./outputs.md#output-frames)
| 64 | DAC_OUTPUT_FLAGS_CH1_ENABLED | If set enables channel 1
| 128 | DAC_OUTPUT_FLAGS_CH2_ENABLED | If set enables channel 2
| 256 | DAC_OUTPUT_FLAGS_CH3_ENABLED | If set enables channel 3
| 512 | DAC_OUTPUT_FLAGS_CH4_ENABLED | If set enables channel 4
| 1024 | DAC_OUTPUT_FLAGS_DONT_LOOP | If set the output will stop when the end of buffer is <br>reached, insted of looping.

## 0x8001 - Keypad data and controlls

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | keypad_polling | int | int delay_milliseconds | Used to set / check if the physical keypad is enabled <br> 0 is dissabled > 0 is enabled, where the value is the <br> number of milliseconds between keypad frames. <br> While -1 just returns the current state and, -2<br> resets the value to defult. <br><br> Note: The hardware "PRG_EXIT" button can not be dissabled <br> as, it uses intrupts
| 1 | uart_keypad_emmulation | int state | int |Used to set / check if the uart keypad emmulation is enabled <br> 0 is dissabled 1 is enabled, -1 just returns the current state and,<br> -2 just sets the value to defult.
| 2 | capture_prg_exit | void* handler | void | Used to set a handler function for "PRG_EXIT", arugment is a <br> pointer to a void(void) function, if null is given, the OS will<br> switch to the defult, witch terminates the current app.
| 3 | get_keypad_state |void | keypad_state | Retruns a [keypad state struct](./keypad.md#keypad-state-struct) with the current state



Note see [keypad.md](./keypad.md) to see more detail about how the keypad works 