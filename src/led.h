// led.h

/**
  * led is a C module for handling all leds
  */


#ifndef _LED_H
#define _LED_H

#include "light_ws2812-2.2/WS2812.h"

const cRGB COLOR_RED =    { /*g*/0,   /*r*/64,  /*b*/0};
const cRGB COLOR_GREEN =  { /*g*/64,  /*r*/0,   /*b*/0};
const cRGB COLOR_BLUE =   { /*g*/0,   /*r*/0,   /*b*/64};
const cRGB COLOR_WHITE =  { /*g*/64,  /*r*/64,  /*b*/64};
const cRGB COLOR_BLACK =  { /*g*/0,   /*r*/0,   /*b*/0};

extern WS2812 LEDS;

typedef enum 
{ 
  // depending on the slot number:
  LED_SLOT_SELECTED,
  LED_SLOT_OPERATION_ACTIVE,
  LED_SLOT_ERROR_FILAMENT_PRESENT,
  LED_SLOT_ERROR_NO_FILAMENT,
  LED_SLOT_SETUP_MENU_ANGLE,
  LED_SLOT_SETUP_MENU_BOWDEN_LEN,
  LED_SLOT_SETUP_ANGLE,
  LED_SLOT_SETUP_BOWDEN_LEN,
  // independent of the slot number:
  LED_ENTER_SLOT_SETUP_MENU,
  LED_DELETE_EEPROM_MENU,
  LED_DELETE_EEPROM_FINISHED
} led_states_t;

void set_led(int slotNumber, cRGB color, bool clearAllBeforeSet = true);
void clr_leds(void);
void set_led_state(int slotNumber, led_states_t state);

#endif //_LED_H
