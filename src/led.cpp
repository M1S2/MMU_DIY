// led.cpp

#include "led.h"
#include "config.h"
#include <avr/io.h>
#include "main.h"

WS2812 LEDS(NUM_SLOTS_MAX);

/**
 * @brief set_led
 * Enable LEDs, active high
 *
 * @param slotNumber: index of the slot for which the LED is set
 * @param color: color for the LED
 * @param clearAllBeforeSet: If true, execute the clr_leds() function before setting the LED
 */
void set_led(int slotNumber, cRGB color, bool clearAllBeforeSet)
{
    if(clearAllBeforeSet) { clr_leds(); }
    _delay_ms(1);
    LEDS.set_crgb_at(slotNumber, color);
	LEDS.sync(); // Sends the value to the LED
}

void clr_leds(void)
{
	for(int i = 0; i < NUM_SLOTS_MAX; i++)
    {
        LEDS.set_crgb_at(i, COLOR_BLACK);
    }
	LEDS.sync(); // Sends the value to the LED
}

void led_blink(int slotNumber)
{
    set_led(slotNumber, COLOR_GREEN);
    _delay_ms(100);
    clr_leds();
    _delay_ms(50);
    set_led(slotNumber, COLOR_GREEN);
    _delay_ms(100);
    clr_leds();
    _delay_ms(50);
}

void set_led_state(int slotNumber, led_states_t state)
{
    switch(state)
    {
    case LED_SLOT_SELECTED:
        set_led(slotNumber, COLOR_GREEN);
        break;
    case LED_SLOT_OPERATION_ACTIVE:
        set_led(slotNumber, COLOR_BLUE);
        break;
    case LED_SLOT_ERROR_FILAMENT_PRESENT:
        set_led(slotNumber, COLOR_RED);
        _delay_ms(100);
        clr_leds();
        _delay_ms(100);
        break;
    case LED_SLOT_ERROR_NO_FILAMENT:
        set_led(slotNumber, COLOR_GREEN);
        _delay_ms(100);
        clr_leds();
        _delay_ms(100);
        break;
    case LED_SLOT_SETUP_MENU_ANGLE:
        set_led(slotNumber, COLOR_WHITE);
        _delay_ms(100);
        clr_leds();
        _delay_ms(900);
        break;
    case LED_SLOT_SETUP_MENU_BOWDEN_LEN:
        set_led(slotNumber, COLOR_WHITE);
        _delay_ms(100);
        clr_leds();
        _delay_ms(75);
        set_led(slotNumber, COLOR_WHITE);
        _delay_ms(100);
        clr_leds();
        _delay_ms(725);
        break;
    case LED_SLOT_SETUP_ANGLE:
        set_led(slotNumber, COLOR_WHITE);
        break;
    case LED_SLOT_SETUP_BOWDEN_LEN:
        set_led(slotNumber, COLOR_WHITE);
        break;
    case LED_ENTER_SLOT_SETUP_MENU:
        clr_leds();
        for (int i = 0; i < numSlots; i++)
        {
            set_led(i, COLOR_WHITE, false);
        }
        _delay_ms(1000);
        clr_leds();
        break;
    case LED_DELETE_EEPROM_MENU:
        clr_leds();
        for (int i = 0; i < numSlots; i++)
        {
            set_led(i, COLOR_RED, false);
        }
        _delay_ms(100);
        clr_leds();
        _delay_ms(200);
        break;
    case LED_DELETE_EEPROM_FINISHED:
        clr_leds();
        for (int i = 0; i < numSlots; i++)
        {
            set_led(i, COLOR_RED, false);
        }
        _delay_ms(1500);
        break;
    }
}
