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
    delay(1);
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
    delay(100);
    clr_leds();
    delay(50);
    set_led(slotNumber, COLOR_GREEN);
    delay(100);
    clr_leds();
    delay(50);
}
