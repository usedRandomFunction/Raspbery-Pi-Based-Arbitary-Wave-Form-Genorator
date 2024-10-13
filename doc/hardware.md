# Raspbery Pi Based Arbitary Waveform Genorator Hardwere

## Overveiw

There are only a few parts to the hardware itself

1. Power supply
2. Power supply Filtering and Regulation
3. Main board
4. The Raspbery Pi
5. DAC modules
6. Keypad
7. LCD


### Power supply

All it does is take mains and give to rails

- 12v  output at about 1.5 amps
- 5.5v output at about 4~5 amps

with reasonable noise levels

### Power supply Filtering and Regulation

This board uses linerer regulators to and LR / C filters to lowwer the noise
while turning the 5.5 in 5 volts. It also creates the +8 and -5v rails for the opamps

### Main boad

Connection point between DACs PI, and keypad combined with the output opamps and connectors.

### The Raspbery Pi

As stated earlyer this project uses a PI 3b

### DAC modules

The current design uses two 74HC595s running above 70 MHz to achive the sample rate,
this is put into a R2R DAC, with relays used to controll the number of bits

### Keypad

Self explaintry, uses SPI to take inputs and give outputs with the exception of PRG_EXIT, which is connected to its own GPIO

### LCD

Uses as the UI for the system


##
## GPIO Connections

<b>`Warning: Contrll data, keypad in and SPI Clock are wrong on revison 1.0 mainboards`</b>

| GPIO | Use |
|------|-----|
|  05  | Keypad input latch
|  06  | Controll data latch
|  07  | SPI CE 1 [Unused, exposed via header]
|  08  | SPI CE 0 [Unused, exposed via header]
|  09  | SPI MISO (keypad in)[Also exposed via header] `WARNING this pin is pulled high at all times and is actived with a pull down`
|  10  | SPI MOSI (Controll data out) [Also exposed via header]
|  11  | SPI Clock [Also exposed via header]
|  13  | PROGRAM_EXIT button
|  14  | UART TX
|  15  | UART RX
|  16  | DAC latch
|  17  | DAC clock
|  18  | Channel 1 LSB
|  19  | Channel 1 MSB
|  20  | Channel 2 LSB
|  21  | Channel 2 MSB
|  22  | Channel 3 LSB
|  23  | Channel 3 MSB
|  24  | Channel 4 LSB
|  25  | Channel 4 MSB
