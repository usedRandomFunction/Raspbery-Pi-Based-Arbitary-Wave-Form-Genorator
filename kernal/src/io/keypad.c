#include "io/keypad.h"

#include "io/hardware_controll_register.h"
#include "run_time_kernal_config.h"
#include "lib/interrupts.h"
#include "lib/timing.h"
#include "lib/events.h"
#include "io/printf.h"
#include "io/uart.h"
#include "io/gpio.h"

#include <stdbool.h>
#include <stddef.h>

static keypad_state s_uart_emmulation_forced_off;
static keypad_state s_uart_emmulation_forced_on;
static keypad_state s_uart_emmulation_standed;
static keypad_state s_physical_keypad_state;
static uint64_t s_prg_exit_last_triggered;
static bool s_uart_emmulation_enabled;
static int s_physical_keypad_delay;

static PRG_EXIT_HANDLER s_prg_exit_handler = &defult_prg_exit_handler;

static void s_tigger_prg_exit_from_gpio(int pin);

#define keypad_input_latch_pin 6


void initialize_keypad()
{
    uart_keypad_emmulation(-2);

    if (allow_physical_keypad)
    {
        gpio_function_select(13, GPFSEL_Input);
        gpio_enable_pin_interupt(13, s_tigger_prg_exit_from_gpio,
            true, false, false, false, false, false);
        gpio_function_select(keypad_input_latch_pin, GPFSEL_Output);    // Set controll pins as outputs

        gpio_set(keypad_input_latch_pin);                               // This one is active low so it gose high

        *hardware_controll_register_keypad_controll_byte = 0x02;        // Start at row 0 (1 << (1 + row))
        hardware_controll_register_write();

        s_physical_keypad_state = 0;
    }

    keypad_polling(-2);
}

int keypad_polling(int delay_milliseconds)
{
    if (allow_physical_keypad == false)
        return 0;

    if (delay_milliseconds == -1)
        return s_physical_keypad_delay / 1000;

    if (delay_milliseconds == -2)
        delay_milliseconds = physical_keypad_default_delay;
    
    if (delay_milliseconds <= 0 || is_running_in_qemu) // Since the emmulator does not understand the hw keypad, we cant work with it
    {   
        disable_irq(30);
        dissable_timer_interrupt();

        s_physical_keypad_delay = 0;

        return 0;
    }
    s_physical_keypad_delay = delay_milliseconds * 1000;

    enable_irq(30);
    enable_timer_interrupt();
    set_timer_interrupt(s_physical_keypad_delay);

    return delay_milliseconds;
}

int uart_keypad_emmulation(int state)
{
    if (allow_uart_keypad_emmulation == false)
        return 0;

    if (state == -1)
        return (int)s_uart_emmulation_enabled;

    if (state == -2)
        state = uart_keypad_emmulation_default_state;


    if (state == 1)
    {
        clear_pending_uart_interupts();
        enable_uart_interupts();
        enable_uart_receive_interupt();

        s_uart_emmulation_enabled = true;

        return 1;
    }

    // If its not a defined state we'll just turn it off

    s_uart_emmulation_enabled = false;

    disable_uart_interupts();
    disable_uart_receive_interupt();

    return 0;
}

void capture_prg_exit(PRG_EXIT_HANDLER handler)
{
    if (handler == NULL)
    {
        s_prg_exit_handler = &defult_prg_exit_handler;

        return;
    }

    s_prg_exit_handler = handler;
}

bool is_using_defult_prg_exit_handler()
{
    return s_prg_exit_handler == &defult_prg_exit_handler;
}

keypad_state get_keypad_state()
{
    keypad_state state = 0;

    if (s_physical_keypad_delay > 0)
        state |= s_physical_keypad_state;

    if (s_uart_emmulation_enabled == false)
        return state;

    state |= s_uart_emmulation_standed;
    s_uart_emmulation_standed = 0;
    state &= ~s_uart_emmulation_forced_off;
    state |= s_uart_emmulation_forced_on;

    return state;
}

void keypad_uart_interupt_handler()
{
    if (s_uart_emmulation_enabled == false)    // Werid error is happening going to check this
        return;

    disable_uart_receive_interupt();

    int number_of_keys = (int)uart_getc();
    bool prg_exit_triggered = false;

    for (int i = 0; i < number_of_keys; i++)
    {
        keypad_state button_bit = 0;
        uint8_t button_code = (uint8_t)uart_getc();
        uint8_t state = (uint8_t)uart_getc();

        switch (button_code)
        {
        case (uint8_t)'0':
            button_bit = KEYPAD_STATE_BUTTON_0;
            break;
        case (uint8_t)'1':
            button_bit = KEYPAD_STATE_BUTTON_1;
            break;
        case (uint8_t)'2':
            button_bit = KEYPAD_STATE_BUTTON_2;
            break;
        case (uint8_t)'3':
            button_bit = KEYPAD_STATE_BUTTON_3;
            break;
        case (uint8_t)'4':
            button_bit = KEYPAD_STATE_BUTTON_4;
            break;
        case (uint8_t)'5':
            button_bit = KEYPAD_STATE_BUTTON_5;
            break;
        case (uint8_t)'6':
            button_bit = KEYPAD_STATE_BUTTON_6;
            break;
        case (uint8_t)'7':
            button_bit = KEYPAD_STATE_BUTTON_7;
            break;
        case (uint8_t)'8':
            button_bit = KEYPAD_STATE_BUTTON_8;
            break;
        case (uint8_t)'9':
            button_bit = KEYPAD_STATE_BUTTON_9;
            break;
        case (uint8_t)'A':
            button_bit = KEYPAD_STATE_BUTTON_A;
            break;
        case (uint8_t)'B':
            button_bit = KEYPAD_STATE_BUTTON_B;
            break;
        case (uint8_t)'C':
            button_bit = KEYPAD_STATE_BUTTON_C;
            break;
        case (uint8_t)'D':
            button_bit = KEYPAD_STATE_BUTTON_D;
            break;
        case (uint8_t)'.':
            button_bit = KEYPAD_STATE_BUTTON_DOT;
            break;
        case (uint8_t)'+':
            button_bit = KEYPAD_STATE_BUTTON_PLUSMINUS;
            break;
        case 0x7F:
            button_bit = KEYPAD_STATE_BUTTON_DEL;
            break;
        case 0x7E:
            button_bit = KEYPAD_STATE_BUTTON_CLR;
            break;
        case 0x7D:
            button_bit = KEYPAD_STATE_BUTTON_ENT;
            break;
        case 0x7C: // PRG_EXIT
            if (state == 1)
                prg_exit_triggered = true;
            break;
        case 0x7B:
            button_bit = KEYPAD_STATE_BUTTON_CH4;
            break;
        case 0x7A:
            button_bit = KEYPAD_STATE_BUTTON_CH3;
            break;
        case 0x79:
            button_bit = KEYPAD_STATE_BUTTON_CH2;
            break;
        case 0x78:
            button_bit = KEYPAD_STATE_BUTTON_CH1;
            break;
        case 0x77:
            button_bit = KEYPAD_STATE_BUTTON_CH4_BUFFER;
            break;
        case 0x76:
            button_bit = KEYPAD_STATE_BUTTON_CH3_BUFFER;
            break;
        case 0x75:
            button_bit = KEYPAD_STATE_BUTTON_CH2_BUFFER;
            break;
        case 0x74:
            button_bit = KEYPAD_STATE_BUTTON_CH1_BUFFER;
            break;

        default:
            break;
        }
    
        keypad_state button_mask = ~button_bit;         // Used when removing the bit form states

        if (button_bit == 0)                            // If we dont under stand this one just skip it
            continue;

        if (state == 0)                                 // "Off" state
        {
            s_uart_emmulation_forced_off &= button_mask;
            s_uart_emmulation_forced_on &= button_mask;
            s_uart_emmulation_standed &= button_mask;
        }
        else if (state == 1)                            // "On" state
        {
            s_uart_emmulation_forced_off &= button_mask;
            s_uart_emmulation_forced_on &= button_mask;
            s_uart_emmulation_standed |= button_bit;
        }
        else if (state == 2)                            // "Forced Off" state
        {
            s_uart_emmulation_forced_off |= button_bit;
            s_uart_emmulation_forced_on &= button_mask;
            s_uart_emmulation_standed &= button_mask;
        }
        else if (state == 3)                            // "Forced On" state
        {
            s_uart_emmulation_forced_off &= button_mask;
            s_uart_emmulation_forced_on |= button_bit;
            s_uart_emmulation_standed &= button_mask;
        }
    }

    enable_uart_receive_interupt();

    if (prg_exit_triggered)
        tigger_prg_exit();
}

void  keypad_poll()
{
    if (is_running_in_qemu)                         // Since the emmulator does not understand the hw keypad, we cant work with it
        return;

    uint8_t recive_buffer[HARDWARE_CONTROLL_REGISTER_SIZE_BYTES];

    uint8_t row_data[] = {0x00, 0x00, 0x00, 0x00};
    // By the power of never deleting these test lines, i keep the bug away
    // static uint64_t last = 0; 
    // uint64_t temp = 0;

    for (int row = 0; row < 4; row++)
    {
        const uint8_t new_target_row = row < 3 ? row + 1 : 0;
        *hardware_controll_register_keypad_controll_byte = 1 << (new_target_row + 1);   // Get the next row ready
        
        wait_cycles(keypad_input_pre_latch_delay);
        gpio_clear(keypad_input_latch_pin);                                             // Save the current state of the keypad
        wait_cycles(keypad_input_latch_delay);
        gpio_set(keypad_input_latch_pin);
        wait_cycles(keypad_input_post_latch_delay);

        hardware_controll_register_write_read(recive_buffer);                           // Read current / set the next row.


        uint8_t raw_data = ~(recive_buffer[0]) << 1;
        uint8_t keypad_data = raw_data & 0b01111110;                                    // Only include bits dirrectly mapped to pins

        // temp <<= 8;
        // temp |= raw_data;

        if (raw_data & 0x80)                                                            // A while ago i had trubble with pin 1 of the 74165
            keypad_data |= 1;                                                           // So i moved it to pin 8, so we need to do this
        
        // temp <<= 8;
        // temp |= keypad_data;
        row_data[row] = keypad_data;
    }
    // if (temp != last)
    //     printf("\n%x\n", temp);

    // last = temp;
    

    s_physical_keypad_state =   ((keypad_state)row_data[3] | 
                                ((keypad_state) row_data[2] << 8) | 
                                ((keypad_state)row_data[1] << 16) | 
                                ((keypad_state)row_data[0] << 24));
}

void keypad_poll_from_timer()
{
    disable_irq(30);
    keypad_poll();

    set_timer_interrupt(s_physical_keypad_delay);
    enable_irq(30);
}

void s_tigger_prg_exit_from_gpio(int pin)
{
    tigger_prg_exit();
}

void tigger_prg_exit()
{
    printf("\nPRG_EXIT interupt raised!\n");

    const uint64_t current_time = get_timer_count();
    const uint64_t delta = current_time - s_prg_exit_last_triggered;
    const int delta_milliseconds = delta / (get_timer_frequency() / 1000);

    if (delta_milliseconds < prg_exit_debounce_time)
    {
        printf("Debounce time has not been reached yet, ignoring\n");
        return;
    }

    s_prg_exit_last_triggered = current_time;


    if (interupt_active)
    {
        printf("Waiting on interupt end before running prg_exit handler\n");

        event_handler_add_interupt_end(s_prg_exit_handler);

        return;
    }
    s_prg_exit_handler();
}

void halt_and_wait_from_user_input()
{
    keypad_state first_state = get_keypad_state();
    printf("Press any key to continue...\n");

    while (get_keypad_state() == first_state) 
    {
        asm volatile ("wfi");               // Sleep untill next input
    }
}
