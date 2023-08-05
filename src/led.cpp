// led.c

#include "led.h"
#include "config.h"
#include <avr/io.h>
#include "main.h"

#include "uart.h"

WS2812 LEDS(10);

/**
 * @brief set_led
 * Enable LEDs, active high
 *
 * @param led: bit mask with
 * // TODO 2: double check documentation
 *   bit0 = green led of last extruder (0)
 *   bit0 = red led of last extruder (0)
 *   alternating green-red until
 *   bit9 = red LED of first extruder (4)
 */
void set_led(uint16_t led) // TODO 2: provide macros with easily readable names
{
    cRGB value;

    for(int i = 0; i < numSlots; i++)
    {
        sendStringToPrinter("set_led");
        
        // slot0 green = 0b01 00 00 00 00
        // slot0 red =   0b10 00 00 00 00
        // slot4 green = 0b00 00 00 00 01
        // slot4 red   = 0b00 00 00 00 10
        bool bit_green = (led & (1 << (2* (numSlots - 1 - i))));
        bool bit_red = (led & (2 << (2 * (numSlots - 1 - i))));

        value.b = 0; value.g = 0; value.r = 0; // RGB Value -> Black

        if(bit_green && bit_red)
        {
            value.b = 128; value.g = 0; value.r = 0; // RGB Value -> Blue
        }    
        else if(bit_green)
        {
            value.b = 0; value.g = 128; value.r = 0; // RGB Value -> Green
        }
        else if(bit_red)
        {
            value.b = 0; value.g = 0; value.r = 128; // RGB Value -> Red
        }
        LEDS.set_crgb_at(i, value);
    }
	LEDS.sync(); // Sends the value to the LED
}

void clr_leds(void)
{
    cRGB value;
    value.b = 0; value.g = 0; value.r = 0; // RGB Value -> Black
	for(int i = 0; i < numSlots; i++)
    {
        LEDS.set_crgb_at(i, value);
    }
	LEDS.sync(); // Sends the value to the LED
}
