# Outputs

This document describes how the outputs work and, how they are controlled.

## Overview

INV-0003 has four separate outputs, referred to as channels 1 though 4.<br>
They (if all optional hardware is added), can be buffered or unbuffered, <br>
and switch between 2, 8, and 16 bit operation.

The buffering is achieved using RC4558 opamps, in unbuffered mode they are<br>
simply bypassed. The bypass can be enabled / disabled individually for each<br>
channel. This is controlled with bits [4, 1] of the [output control IC](#output-control-ic), for<br>
channels [1, 4] respectively. Where `1` Is bypassed and `0` is not bypassed.

The DAC resolution is also controlled by the [output control IC](#output-control-ic), however<br>
All outputs run at the same resolution and can not be edited  individually. <br>
More detail is given in the [DAC](#dac) section.

The presence or lack off, of hardware features is communicated to the kernel<br>
by the [channels respective configuration file](#configuration-files).

### DAC

The four outputs are driven by 2/8/16 bit variable resolution 2R2 DACs,<br>
given the part number INV-0003-2. They consist of two 74HC595’s, three<br>
G5V-1-12V relays and the R2R DAC, using 1 and 2 KΩ resistors.

While the two ICs are called MSB and LSB, that naming convention, is not<br>
entirely true. Since it takes time to shift bits in data to the registers,<br>
It is wired so the data is split between the shift register’s pins evenly.<br>
This means that the outputs and data shifted in are different.

| DAC bit | IC  | IC output bit |
|---------|-----|---------------|
| 0       | LSB | 0             |
| 1       | MSB | 0             |
| 2       | LSB | 1             |
| 3       | LSB | 2             |
| 4       | LSB | 3             |
| 5       | MSB | 1             |
| 6       | MSB | 2             |
| 7       | MSB | 3             |
| 8       | LSB | 4             |
| 9       | LSB | 5             |
| 10      | LSB | 6             |
| 11      | LSB | 7             |
| 12      | MSB | 4             |
| 13      | MSB | 5             |
| 14      | MSB | 6             |
| 15      | MSB | 7             |

While pins 4 and 5 of the output control IC are connected to `High Speed Enable`<br>
 and `16 Bit Mode Enable` respectively, setting resolution is bit more <br>
complicated, but can be described in the following table: 

|  Resolution |  High Speed Enable |  16 Bit Mode Enable |
|-------------|--------------------|---------------------|
| 2-bit       | `1`                | `1`                 |
| 8-bit       | `0`                | `0`                 |
| 16-bit      | `0`                | `1`                 |

### Output control IC

The output control IC is 74HC595, which is connected to a ULN2003A to let it<br>
control relays. It is connected to the SPI MOSI and clock lines, using GPIO 6<br>
as the latch signal. 6 out of 8 bits are used and are as follows:

| Output bit | Connection |
|------------|------------|
| 0          | Channel 4 Disengage buffering |
| 1          | Channel 3 Disengage buffering |
| 2          | Channel 2 Disengage buffering |
| 3          | Channel 1 Disengage buffering |
| 4          | High Speed Enable  |
| 5          | 16 Bit Mode Enable |

Note that user apps cannot directly interface with this IC but instead call<br>
system calls to configure the channels. During these system calls the kernel<br>
checks for hardware support of the configuration and sets the values accordingly.

## Configuration files

There are `5` configurations files for the outputs.

- outputs.cfg
- channel`N`.cfg where `N` is 1 through 4 inclusive.

### outputs.cfg

The config file `outputs.cfg` stores the sample rates of the output at different resolutions.<br>
As a result it has the following entries:

- SAMPLE_RATE_2_BIT
- SAMPLE_RATE_8_BIT
- SAMPLE_RATE_16_BIT

Each of these entries contains the sample rate at the given resolutions in hertz as a integer.<br>
If a value of `0` is given this is interpreted as the  resolution not being supported, while<br>
a value of `0xFFFFFFFF` may be used to signify a enabled channel, with a unknown sample<br>
rate. i.e not measured yet. If a value is not included or the file cant be loaded al, values<br>
defult to zero.

### channel`N`.cfg

The `channelN.cfg` files, represent the hardware support of diffrent resolution and<br>
buffering. There is one file per channel so `channel1.cfg` only represents channel 1,<br>
while `channel2.cfg`  represents channel 2 and so on. They contain the following entries:

- SUPPORTS_RESOLUTION_2_BIT
- SUPPORTS_RESOLUTION_8_BIT
- SUPPORTS_RESOLUTION_16_BIT
- SUPPORTS_STATE_UNBUFFERED
- SUPPORTS_STATE_BUFFERED

For each of these entries a value of `0` represents unsupported, while a value `> 0`<br>
represents supported. All of these values default to zero if they are not included,<br>
or if the file cant be loaded.

## Transmission and storage in memory

The inputs of the DACs are connected to the Pis GPIO, latch and clock are 16 and<br>
17 respectively. From there it goes 18 and 19, being channel 1 LSB and MSB, then<br>
20 and 21 for channel 2 LSB and MSB, 22 and 23 for channel 3, 24 and 25 for channel 4.

Including the order required by the DAC, the data lines for one channel look like this:

| LSB | MSB | |
|-----|-----|-|
|  0  |  1  | 2-Bit |
|  2  |  5  |
|  3  |  6  |
|  4  |  7  | 8-Bit |
|  8  | 12  |
|  9  | 13  |
| 10  | 14  |
| 11  | 15  | 16-Bit |

In this table the numbers reefer to what bit of the desired output is to be outputted.<br>
The table is read from the bottom up, such that the lowest value is to be transmitted<br>
first,and the highest value last. The third row is where to start reading it for a given<br>
desired resolution.

Since the AWG can only write two bits at a time and needs to be fast, with the goal being,<br>
10 mega samples a second. The output needs to be fast, even with a system clock in the Ghz,<br>
this is still a challenge, mostly due to memory bandwith. As a result the output on the AWG<br>
(At a high level) works like this:

1. Load the next 64 bits from the buffer
2. Use a bitmask to get the data bits to set
3. Output these bits and shift them into the registers
4. Bit shift loaded value
4. Repeat [2, 4] until the entry value has been outputted
5. Latch
6. Repeat [2, 5] until all the values in the loaded number have been used
7. Repeat [1, 6] until the end of the buffer is reached
8. Go to the start of the buffer and restart the loop

Note these lines form the `shift_register_out` macro in kernel/src/io/data_output.c

``` ini
*clr_address = clock_mask | (data_mask & ~(current_data)); 
*set_address = data_mask & (current_data);
*set_address = clock_mask;
```

See how `current_data` is anded with a bit mask and then writen to set / clr addresses?<br>
these addresses directly set or clear GPIO pins. `current_data` is the 64 bits from the buffer.<br>
This is why the output pins are all next to each other (looking from pin numbers at least) going<br>
LSB, MSB, LSB…

Now with that out of the way we can get into how the buffer is made

### Output frames

A output frame stores what data lines go high and what go low, and is how the table form the start<br>
of this section is used. The size of output frames is based of which channels are enabled. While not just<br>
the number of enabled channels it can be generalised as `size = (highest_ channel – lowest_channel) * 2 + 2`<br>
where `highest` and `lowest` are just based the channel numbers of the enabled channels. However a size of `6`<br>
bits is not possible, in the case size equals 6 channels `1` and `4` are used as the lowest and highest channel.

For example if just channel `1` is enabled the size of a frame is `2` bits. If it was `1`, `2`, and `4` the <br>
size if `8` bits but if it was just `1` and `4` the size if still `8`.

Starting with the lowest channel in the frame, the LSB and MSB are stored, then LSB and MSB of the next channel,<br>
etc. If a channel is disabled but still in the frame, its values are ignored.<br>
Shown is the example of channel `1` and `3` being enabled:

| Frame Bit |    0     |    1     |    2     |    3     |     4    |    5     |
|-----------|----------|----------|----------|----------|----------|----------|
| Use       | Ch 1 LSB | Ch 1 MSB | Ignored  | Ignored  | Ch 3 LSB | Ch 3 MSB |

### Output buffer construction

As stated earlier the buffer is made out of the 64-bit numbers. These numbers store 8 to 32 frames depending on<br>
their size. `32` Frames for `2` bit, `16` for `4` bit and `8` for `8` bit respectively. The frames are stored next<br>
to each other with the frames to be transmitted latter being shifted more and more left. The amount of left shift, <br>
is `size * n` where n is the position in the buffer, again `0` is transmitted first then `1` then `2` etc.

This value need to be shifted again since this value will be directly used to control on the GPIO pins.<br>
The size of this additional start offset is based of the `lowest_channel` described in the output frames section<br>
(Including the case where `1` is used no mater the actual lowest enabled channel). A offset of `18` is used for<br>
channel `1`, `20` for channel `2`, `22` for channel `3` and `24` for channel `4`

