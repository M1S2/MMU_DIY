// led.c

#include "led.h"
#include "config.h"
#include <avr/io.h>

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
    //led = ((led & 0x00ff) << 8) | ((led & 0x0300) >> 2);
    //shr16_write(shr16_v | led);
}

void clr_leds(void)
{
    //shr16_write(shr16_v & ~SHR16_LED_MSK);
}
