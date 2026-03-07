#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*PRG_EXIT_HANDLER)(void);

typedef uint32_t keypad_state;

// Initializes the keypad and keypad emmulation to defult values from system.cfg
void initialize_keypad();

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
void capture_prg_exit(PRG_EXIT_HANDLER handler);

// Checks if the handler if defult
// @return true if s_prg_exit_handler == defult_prg_exit_handler else false
bool is_using_defult_prg_exit_handler();

// Gets the current state of the keypad
// @return The current state of the keypad as a keypad_state struct
keypad_state get_keypad_state();

// The defult handler for the PRG_EXIT interupt,
// @warning Is not included by defult
void defult_prg_exit_handler();

// Messures the current state of the physcial keypad
void  keypad_poll();

// Used by the keypad to handle UART interupt
// @warning Is not to be called dirrectly and will halt exication untill it has recived all data
void keypad_uart_interupt_handler();

// Messures the current state of the physcial keypad
// @warning This is to be called by the timer interupt not user functions
void  keypad_poll_from_timer();

// Forces the PRG_EXIT interupt to run
void tigger_prg_exit();

// Function will block exicution untill keypad state changes
void halt_and_wait_from_user_input();

enum
{
    KEYPAD_STATE_BUTTON_7           = (1 << (0 + 0 * 8)),
    KEYPAD_STATE_BUTTON_8           = (1 << (0 + 1 * 8)),
    KEYPAD_STATE_BUTTON_9           = (1 << (0 + 2 * 8)),
    KEYPAD_STATE_BUTTON_A           = (1 << (0 + 3 * 8)),
    KEYPAD_STATE_BUTTON_4           = (1 << (1 + 0 * 8)),
    KEYPAD_STATE_BUTTON_5           = (1 << (1 + 1 * 8)),
    KEYPAD_STATE_BUTTON_6           = (1 << (1 + 2 * 8)),
    KEYPAD_STATE_BUTTON_B           = (1 << (1 + 3 * 8)),
    KEYPAD_STATE_BUTTON_1           = (1 << (2 + 0 * 8)),
    KEYPAD_STATE_BUTTON_2           = (1 << (2 + 1 * 8)),
    KEYPAD_STATE_BUTTON_3           = (1 << (2 + 2 * 8)),
    KEYPAD_STATE_BUTTON_C           = (1 << (2 + 3 * 8)),
    KEYPAD_STATE_BUTTON_DOT         = (1 << (3 + 0 * 8)),
    KEYPAD_STATE_BUTTON_0           = (1 << (3 + 1 * 8)),
    KEYPAD_STATE_BUTTON_PLUSMINUS   = (1 << (3 + 2 * 8)),
    KEYPAD_STATE_BUTTON_D           = (1 << (3 + 3 * 8)),
    KEYPAD_STATE_BUTTON_DEL         = (1 << (4 + 0 * 8)),
    KEYPAD_STATE_BUTTON_CLR         = (1 << (4 + 1 * 8)),
    KEYPAD_STATE_BUTTON_ENT         = (1 << (4 + 2 * 8)),
    //                  PRG_EXIT Not included in button state struct
    KEYPAD_STATE_BUTTON_CH1         = (1 << (5 + 0 * 8)),
    KEYPAD_STATE_BUTTON_CH2         = (1 << (5 + 1 * 8)),
    KEYPAD_STATE_BUTTON_CH3         = (1 << (5 + 2 * 8)),
    KEYPAD_STATE_BUTTON_CH4         = (1 << (5 + 3 * 8)),
    KEYPAD_STATE_BUTTON_CH1_BUFFER  = (1 << (6 + 0 * 8)),
    KEYPAD_STATE_BUTTON_CH2_BUFFER  = (1 << (6 + 1 * 8)),
    KEYPAD_STATE_BUTTON_CH3_BUFFER  = (1 << (6 + 2 * 8)),
    KEYPAD_STATE_BUTTON_CH4_BUFFER  = (1 << (6 + 3 * 8))
};

#endif
