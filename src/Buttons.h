// Buttons.h

#ifndef _BUTTONS_h
#define _BUTTONS_h

#include <stdint.h>
#include "Arduino.h"

#define BTN_MODIFIER_LONG_PRESS 8
#define BTN_MODIFIER_NONE       0

#define BTN_RIGHT               4
#define BTN_MIDDLE              2
#define BTN_LEFT                1
#define BTN_NONE                0

uint8_t buttonClicked();

#endif //_BUTTONS_h
