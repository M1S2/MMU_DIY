// led.h

/**
  * led is a C module for handling all leds
  */


#ifndef _LED_H
#define _LED_H

#include <inttypes.h>
#include "light_ws2812-2.2/WS2812.h"

const cRGB COLOR_RED =    { /*g*/0,   /*r*/64,  /*b*/0};
const cRGB COLOR_GREEN =  { /*g*/64,  /*r*/0,   /*b*/0};
const cRGB COLOR_BLUE =   { /*g*/0,   /*r*/0,   /*b*/64};
const cRGB COLOR_WHITE =  { /*g*/64,  /*r*/64,  /*b*/64};
const cRGB COLOR_BLACK =  { /*g*/0,   /*r*/0,   /*b*/0};

extern WS2812 LEDS;

void set_led(int slotNumber, cRGB color, bool clearAllBeforeSet = true);
void clr_leds(void);
void led_blink(int slotNumber);

#endif //_LED_H
