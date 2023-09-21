// Buttons.h

#ifndef _BUTTONS_H
#define _BUTTONS_H

#include <stdint.h>

// !!! Key Pins must already be set as inputs and Pull-Ups must be enabled !!!
#define KEY_PIN         PIND
#define KEY_LEFT		3	// PD3
#define KEY_MIDDLE	    4	// PD4
#define KEY_RIGHT       5   // PD5

#define ALL_KEYS        (1<<KEY_RIGHT | 1<<KEY_MIDDLE | 1<<KEY_LEFT)

#define REPEAT_MASK     ALL_KEYS		    // repeat: all keys
#define REPEAT_START    50					// after 500ms
#define REPEAT_NEXT     50					// every 500ms

void initButtonPins();
void initButtonTimer();

void debounce_timer_interrupt();
uint8_t get_key_press(uint8_t key_mask);
uint8_t get_key_rpt(uint8_t key_mask);
uint8_t get_key_state(uint8_t key_mask);
uint8_t get_key_short(uint8_t key_mask);
uint8_t get_key_long(uint8_t key_mask);

#endif //_BUTTONS_H
