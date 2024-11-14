#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

typedef uint32_t keypad_state;

// TODO hardware keypad and GPIO interupt

// Initializes the keypad and keypad emmulation to defult values from system.cfg
void keypad_init();

// Used to set / check if the physical keypad is enabled
// @param delay_milliseconds When > 0 this is the delay between keypad frames, when = 0 it turns off the physical keypad when -1 just returns the current state and -2 just sets the value to defult from (system.cfg).
// @return The new / current delay between keypad frames or -1 if disabled
int keypad_polling(int delay_milliseconds);

// Used to set / check if the uart keypad emmulation is enabled
// @param state 0 is dissabled 1 is enabled, -1 just returns the current state  -2 just sets the value to defult from (system.cfg).
// @return The new / current state of keypad emmulation
int uart_keypad_emmulation(int state);

// Used to set the a handler function for "PRG_EXIT"
// @param handler a pointer to a void (void) function to use as the handler
// @note is handler is NULL this will switch to the defult handler (void defult_prg_exit_handler()))
void capture_prg_exit(void* handler);

// Gets the current state of the keypad
// @return The current state of the keypad as a keypad_state struct
keypad_state get_keypad_state();

// The defult handler for the PRG_EXIT interupt,
// @warning Is not included by defult
void defult_prg_exit_handler();

// Used by the keypad to handle UART interupt
// @warning Is not to be called dirrectly and will halt exication untill it has recide all data
void keypad_uart_interupt_handler();

// Forces the PRG_EXIT interupt to run
void tigger_prg_exit();

enum
{
    KEYPAD_STATE_BUTTON_7           = (1 << 0),
    KEYPAD_STATE_BUTTON_8           = (1 << 1),
    KEYPAD_STATE_BUTTON_9           = (1 << 2),
    KEYPAD_STATE_BUTTON_A           = (1 << 3),
    KEYPAD_STATE_BUTTON_4           = (1 << 4),
    KEYPAD_STATE_BUTTON_5           = (1 << 5),
    KEYPAD_STATE_BUTTON_6           = (1 << 6),
    KEYPAD_STATE_BUTTON_B           = (1 << 7),
    KEYPAD_STATE_BUTTON_1           = (1 << 8),
    KEYPAD_STATE_BUTTON_2           = (1 << 9),
    KEYPAD_STATE_BUTTON_3           = (1 << 10),
    KEYPAD_STATE_BUTTON_C           = (1 << 11),
    KEYPAD_STATE_BUTTON_DOT         = (1 << 12),
    KEYPAD_STATE_BUTTON_0           = (1 << 13),
    KEYPAD_STATE_BUTTON_PLUSMINUS   = (1 << 14),
    KEYPAD_STATE_BUTTON_D           = (1 << 15),
    KEYPAD_STATE_BUTTON_DEL         = (1 << 16),
    KEYPAD_STATE_BUTTON_CLR         = (1 << 17),
    KEYPAD_STATE_BUTTON_ENT         = (1 << 18),
    //                  PRG_EXIT Not included in button state struct
    KEYPAD_STATE_BUTTON_CH1         = (1 << 20),
    KEYPAD_STATE_BUTTON_CH2         = (1 << 21),
    KEYPAD_STATE_BUTTON_CH3         = (1 << 22),
    KEYPAD_STATE_BUTTON_CH4         = (1 << 23),
    KEYPAD_STATE_BUTTON_CH1_BUFFER  = (1 << 24),
    KEYPAD_STATE_BUTTON_CH2_BUFFER  = (1 << 25),
    KEYPAD_STATE_BUTTON_CH3_BUFFER  = (1 << 26),
    KEYPAD_STATE_BUTTON_CH4_BUFFER  = (1 << 27)
};

#endif