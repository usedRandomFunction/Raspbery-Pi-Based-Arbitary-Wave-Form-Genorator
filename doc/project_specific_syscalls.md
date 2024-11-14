# Project specific syscalls

The project specific syscalls follow the standard as defined in the [v0 abi](./v0_abi.md)

## syscall tables vs SVC argument

## 0xFF00 - Reserved for hardware outputs (DAC) and settings (buffering / resulation)


## 0xFF01 - Keypad data and controlls

| w8 Value | Name | Arguements | Return | Description |
|----------|------|------------|--------|-------------|
| 0 | keypad_polling | int | int | Used to set / check if the physical keypad is enabled <br> 0 is dissabled > 0 is enabled, where the value is the <br> number of milliseconds between keypad frames. <br> While -1 just returns the current state and, -2<br> resets the value to defult. <br><br> Note: The hardware "PRG_EXIT" button can not be dissabled <br> as, it uses intrupts
| 1 | uart_keypad_emmulation | int | int |Used to set / check if the uart keypad emmulation is enabled <br> 0 is dissabled 1 is enabled, -1 just returns the current state and,<br> -2 just sets the value to defult.
| 2 | capture_prg_exit | void* | void | Used to set a handler function for "PRG_EXIT", arugment is a <br> pointer to a void(void) function, if null is given, the OS will<br> switch to the defult, witch terminates the current app.
| 3 | get_keypad_state |void | keypad_state | Retruns a [keypad state struct](./keypad.md#keypad-state-struct) with the current state



Note see [keypad.md](./keypad.md) to see more detail about how the keypad works 