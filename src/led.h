// led.h

/**
  * led is a C module for handling all leds
  */


#ifndef _LED_H
#define _LED_H

#include <inttypes.h>
#include "light_ws2812-2.2/WS2812.h"

extern WS2812 LEDS;

void set_led(uint16_t led);
void clr_leds(void);

#endif //_LED_H
