#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "led.h"
#include "uart.h"
#include "spi.h"
#include "mmctl.h"
#include "motion.h"
#include "Buttons.h"
#include "SetupMenu.h"
#include "permanent_storage.h"
#include "slotDetection.h"
#include "config.h"
#include "uart.h"
#include <avr/wdt.h>
#include <inttypes.h>

// signals from interrupt to main loop
extern bool inErrorState;
extern int numSlots;
extern long startWakeTime;

void fixTheProblem(bool showPrevious = false);

#endif //_MAIN_H
