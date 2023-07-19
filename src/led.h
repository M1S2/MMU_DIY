// led.h

/**
  * led is a C module for handling all leds
  */


#ifndef _LED_H
#define _LED_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

void set_led(uint16_t led);
void clr_leds(void);

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)
#endif //_LED_H
