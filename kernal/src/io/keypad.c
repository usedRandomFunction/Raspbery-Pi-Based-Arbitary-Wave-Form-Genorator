#include "io/keypad.h"

#include "run_time_kernal_config.h"
#include "lib/interrupts.h"
#include "io/uart.h"

#include <stdbool.h>
#include <stddef.h>

static keypad_state s_uart_emmulation_forced_off;
static keypad_state s_uart_emmulation_forced_on;
static keypad_state s_uart_emmulation_standed;
static bool s_uart_emmulation_enabled;

typedef void (*PRG_EXIT_HANDLER)(void);

static PRG_EXIT_HANDLER prg_exit_handler = &defult_prg_exit_handler;

void keypad_init()
{
    uart_keypad_emmulation(-2);
    keypad_polling(-2);
}


int keypad_polling(int delay_milliseconds)
{
    // TODO hardware keypad and GPIO interupt

    return 0;
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
        enable_irq(57);
        enable_uart_interupts();
        enable_uart_receive_interupt();

        s_uart_emmulation_enabled = true;

        return 1;
    }

    // If its not a defined state we'll just turn it off

    s_uart_emmulation_enabled = false;

    disable_irq(57);
    disable_uart_interupts();
    disable_uart_receive_interupt();

    return 0;
}

void capture_prg_exit(void* handler)
{
    if (handler == NULL)
    {
        prg_exit_handler = &defult_prg_exit_handler;

        return;
    }

    prg_exit_handler = (PRG_EXIT_HANDLER)handler;
}

keypad_state get_keypad_state()
{
    keypad_state state = 0;

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

void tigger_prg_exit()
{
    prg_exit_handler();
}