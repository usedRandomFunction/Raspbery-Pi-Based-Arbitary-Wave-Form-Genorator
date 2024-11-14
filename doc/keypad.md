# AWG Keypad

The keypad within the AWG exists as a physical keypad and a uart emmulation. <br>
Both can be toggleed on / off, with the exception of the hardware [prg_exit](#prg_exit) button.

## Physical keypad

The layout of the physical keyboard is as shown below

| <center> Row / <br> Column </center> | <center> 0 </center> | <center> 1 </center> | <center> 2 </center> | <center> 3 </center> |
|--------------------------------------|---|---|---|---|
| <center> 0 </center> | <center> 7 </center>| <center> 8 </center> | <center> 9 </center> | <center> A </center> |
| <center> 1 </center> | <center> 4 </center>| <center> 5 </center> | <center> 6 </center> | <center> B </center> |
| <center> 2 </center> | <center> 1 </center>| <center> 2 </center> | <center> 3 </center> | <center> C </center> |
| <center> 3 </center> | <center> . </center>| <center> 0 </center> | <center> ± </center> | <center> D </center> |
| <center> 4 </center> | <center> DEL </center> | <center> CLR </center> | <center> ENT </center> | <center> PRG_EXIT </center> |
| <center> 5 </center> | <center> CH1 </center> | <center> CH2 </center> | <center> CH3 </center> | <center> CH4 </center> |
| <center> 6 </center> | <center> BUF </center> | <center> BUF </center> | <center> BUF </center> | <center> BuF </center> |

All buttons except for [PRG_EXIT](#prg_exit) are polled, (prg_exit is on a gpio interupt).<br>
The polling function checks all four columns, to create a keypad "frame" of what buttons are held.<br>
The time (In milliseconds) between these frames is controlled by `int keypad_polling(int)`.

## UART emmulation

When the UART emmulation is enabled, every UART input needs to follow the standered:

| Byte | Value |
|------|-------|
| 0 | The number of keys included in the update
| > 0 | The entrys them self

### Entry

| Byte | Value |
|------|-------|
| 0 | The button to update see [button codes](#button-codes)|
| 1 | The new state see [states](#states)|

#### Button codes

| Button | Value |
|--------|-------|
| `0` through `9` | 0x30 - 0x39 (char `0` - `9`) |
| `A` through `D` | 0x41 - 0x44 (char `A` - `D`) |
| `.` | 0x2E (char `.`) |
| `±` | 0x2B (char `+`) |
| `DEL` | 0x7F |
| `CLR` | 0x7E |
| `ENT` | 0x7D |
| `PRG_EXIT` | 0x7C |
| `CH1` though `CH4` | 0x78 - 0x7B (0x78 is given to `CH1` <br> then 0x75 is `CH2` ect) |
| `BUF` | 0x74 - 0x77 (0x74 is given to `BUF` in<br> column 0 then 0x75 in column 1... ect) |

#### States

| Value | Description |
|-------|-------------|
| 0     | Button is in the "off" state,<br> This does not overide the physical keypad
| 1     | Button is in the "on" state,<br> This does not overide the physical keypad
| 2     | Button is in the "forced_off" state,<br> This does overide the physical keypad
| 3     | Button is in the "forced_on" state,<br> This does overide the physical keypad

Note on settings that do not overide the physical keypad, only apply in the next input frame. <br>
In that frame, if a button is in the "off" state just use the physical keypad, for the "on" state <br> 
it sets the button to on. Note that if it is set to "off" before the program grabs the frame, <br>
the input is canceled.In "forced" states the button will have that value in all frames, <br> 
untill the state is changed again. <br>
This has one exception, PRG_EXIT will only accept "on" or "off".

## Reading the keypad with code

Reading the keypad's state is simple and is completed with `keypad_state get_keypad_state()`.<br>
This returns the current state of the keypad, as a keypad_state struct.


PRG_EXIT is the only exception as it is a interrupt.

### PRG_EXIT

The PRG_EXIT interupt is triggered when the button switches from off to on.<br>
By defult this interrupt, will close the current program but can be captured by<br>
the current program if needed. This is controlled by `void capture_prg_exit(void* handler)`.<br>
`void* handler` is a pointer to a `void (void)` function that is to be called, <br>
when the interupt is triggered. To reset the interupt back to the defult state, <br>
call `capture_prg_exit(NULL)`.

### keypad state struct

In reallity the keypad state struct is just a `uint32_t`, however its bits have a meaning.<br>
For each button on the [Physical keypad](#physical-keypad), a bit is mapped to the button, where `'1'` is on and<br>
`'0'` if off. The offset of this bit is `column + row * 8`, with exception of PRG_EXIT, which <br>
is not included. Any bits not given a definition by this standered should be set to `'0'`

## Note about button meanings

During numeric input ABC, ± have defined meanings:

| Button | Meaning in numeric input |
|--------|--------------------------|
| A | x10^±6 |
| B | x10^±3 |
| C | x10^0 |
| ± | On the start of input / if the last button was not A,B,C, it toggles the sign <br>Of the number, If the last input was A or B it toggles between the ± state
| D | Escape |

In non-numeric input some numbers double as the arrow keys

| Button | Meaning in non-numeric input |
|--------|------------------------------|
| 8 | Up arrow |
| 6 | Right arrow|
| 4 | Left arrow |
| 2 | Down arrow |
