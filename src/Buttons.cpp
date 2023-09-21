//! @file

/************************************************************************/
/*                                                                      */
/*                      Debouncing 8 Keys                               */
/*                      Sampling 4 Times                                */
/*                      With Repeat Function                            */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                      danni@specs.de                                  */
/*                                                                      */
/*			http://www.mikrocontroller.net/articles/Entprellung			*/
/************************************************************************/

#include "Buttons.h"
#include "config.h"
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t key_state;                                // debounced and inverted key state:
// bit = 1: key pressed
volatile uint8_t key_press;                                // key press detect
volatile uint8_t key_rpt;                                  // key long press and repeat

/// @brief Setup all button pins to be inputs and enable the pullups
void initButtonPins()
{
    pinMode(PIN_BTN_LEFT, INPUT);
    pinMode(PIN_BTN_MIDDLE, INPUT);
    pinMode(PIN_BTN_RIGHT, INPUT);
    pullup(PIN_BTN_LEFT);
    pullup(PIN_BTN_MIDDLE);
    pullup(PIN_BTN_RIGHT);
}

/// @brief Init the 8-bit Timer0 used for button handling
void initButtonTimer()
{
    TCCR0 = (1 << CS02) | (1 << CS00) | (1 << WGM01);       // Set Prescaler to 1024, this implicitly enables the timer; set the timer to CTC mode (clear on compare match)
    OCR0 = (uint8_t)((F_CPU / (float)1024) * 10e-3 + 0.5);  // Set output compare register to trigger after 10 ms
    TIMSK |= (1 << OCIE0);                                  // Enable compare match interrupt
}

/// @brief ISR for the Timer0 Compare match. This timer is used for button handling.
ISR(TIMER0_COMP_vect)
{
	debounce_timer_interrupt();
}

/// @brief Function called by the Timer0 Compare match ISR
void debounce_timer_interrupt()						//every 10 ms
{
	static uint8_t ct0 = 0xFF, ct1 = 0xFF, rpt;
	uint8_t i;
	
	i = key_state ^ ~KEY_PIN;                       // key changed ?
	ct0 = ~( ct0 & i );                             // reset or count ct0
	ct1 = ct0 ^ (ct1 & i);                          // reset or count ct1
	i &= ct0 & ct1;                                 // count until roll over ?
	key_state ^= i;                                 // then toggle debounced state
	key_press |= key_state & i;                     // 0->1: key press detect
	
	if( (key_state & REPEAT_MASK) == 0 )            // check repeat function
	rpt = REPEAT_START;                          	// start delay
	if( --rpt == 0 ){
		rpt = REPEAT_NEXT;                        	// repeat delay
		key_rpt |= key_state & REPEAT_MASK;
	}
}

/// @brief Check if a key has been pressed. Each pressed key is reported only once
/// @param key_mask Bitmask with 1s for all buttons to check 
/// @return Mask with 1s for all keys pressed
uint8_t get_key_press( uint8_t key_mask )
{
	cli();                                          // read and clear atomic !
	key_mask &= key_press;                          // read key(s)
	key_press ^= key_mask;                          // clear key(s)
	sei();
	return key_mask;
}

/// @brief Check if a key has been pressed long enough such that the key repeat functionality kicks in.
///
/// After a small setup delay the key is reported being pressed in subsequent calls to this function.
/// This simulates the user repeatedly pressing and releasing the key.
///
/// @param key_mask Bitmask with 1s for all buttons to check
/// @return Mask with 1s for all keys that triggered the repeat feature
uint8_t get_key_rpt( uint8_t key_mask )
{
	cli();                                          // read and clear atomic !
	key_mask &= key_rpt;                            // read key(s)
	key_rpt ^= key_mask;                            // clear key(s)
	sei();
	return key_mask;
}

/// @brief Check if a key is pressed right now
/// @param key_mask Bitmask with 1s for all buttons to check
/// @return Mask with 1s for all keys pressed right now
uint8_t get_key_state( uint8_t key_mask )
{
	key_mask &= key_state;
	return key_mask;
}

/// @brief Detect if the key(s) at the mask 1s was pressed short
/// @param key_mask Bitmask with 1s for all buttons to check
/// @return Mask with 1s for all pressed keys
uint8_t get_key_short( uint8_t key_mask )
{
	cli();                                          // read key state and key press atomic !
	return get_key_press( ~key_state & key_mask );
}


/// @brief Detect if the key(s) at the mask 1s was pressed long
/// @param key_mask Bitmask with 1s for all buttons to check
/// @return Mask with 1s for all pressed keys
uint8_t get_key_long( uint8_t key_mask )
{
	return get_key_press( get_key_rpt( key_mask ));
}