# configuration files (.cfg)

The config files are based of .ini files and are simuler except:
- They do not have sections i.e [SECTION name]

## format

'#' represents a comment

KEY_NAME = value

thats almost it

keys can not have spaces, values stop on new lines
unless quotation marks are used

keys and valyes need to be seporated by ether '=' ' ' or a combination of these two

## Sytem configuration files

there currently is only one system config file `system.cfg` and it is as follows

| Name | Description | Defult Value |
|------|-------------|--------------|
| `MAIN_INTERFACE_APP` | Tells the kernal what app to open on start up <br><br>Note: that if the program exits in less then 0.25 seconds <br>The kernal will "abort" and stop loading the app otherwise it would loop | <center> N/A </center> |
| `ALLOW_PHYSICAL_KEYPAD` | Tells the kernal whether or not to allow the physical keypad to be enabled.<br>If set to 0 the physical keypad is completly dissabled, 1 enables it. This includs PRG_EXIT | <center> 1 </center> |
| `PHYSICAL_KEYPAD_DEFAULT_DELAY` | Tells the kernal the defult delay between keypad frames (in milliseconds),<br>0 dissables the keypad/| <center> 50 </center> |
| `ALLOW_UART_KEYPAD_EMMULATION` | Tells the kernal whether or not to allow the uart keypad emmulation to be enabled.<br>If set to 0 the uart keypad emmulation is completly dissabled, 1 enables it. | <center> 0 </center> |
| `UART_KEYPAD_EMMULATION_DEFAULT_STATE` | Tells the kernal what the defult state of uart keypad emmulation is.<br>If set to 0 the uart keypad emmulation is dissabled de defult, 1 enables it. | <center> 0 </center> |
| `SPI_CLOCK_FREQUENCY` | Tells the kernal what frequency, to run SPI0 at. The value is provided in hertz | <center>30 MHz</center>
| `PRG_EXIT_DEBOUNCE_TIME`| Adds a "cooldown" to the PRG_EXIT interupt, to help with debounce. The given value<br>decides the minimum time between interupts in milliseconds | <center>200</center> 
| `ALLWAYS_SHIRNK_FRAME_BUFFER_IF_POSSIBLE` | States wherever or not, the frame buffer should shink if possible applys when reducing<br> the number of virtual buffers. A value `> 0` represents `true`, while `0` represents `false` | <center><i>see kconfig.h</i></center>
| `MAXIMUM_NUMBER_OF_FRAME_BUFFERS` | Stores the maximum allowed number of virtual frame buffers | <center><i>see kconfig.h</i></center>
| `MINIMUM_NUMBER_OF_FRAME_BUFFERS` | Stores the minimum allowed number of virtual frame buffers | <center><i>see kconfig.h</i></center>
| `DISPLAY_HEIGHT` | Stores the height of the display in pixels | <center><i>see kconfig.h</i></center>
| `DISPLAY_WIDTH` | Stores the width of the display in pixels | <center><i>see kconfig.h</i></center>
| `IS_RUNNING_IN_QEMU` | Used to tell the kernal if its running in qemu as some functions will hang if qemu<br>0 is not on qemu, any other value is qemu | <center> 0 </center> |

Note: Defult value is the value assemed by the kernel if it cannot find the entry while loading<br>the config file. If `N/A` is given the kernel will refuse to load if the entry is not pressent.
