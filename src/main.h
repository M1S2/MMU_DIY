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
#include "permanent_storage.h"
#include "config.h"
#include "uart.h"
#include <avr/wdt.h>
#include <inttypes.h>

void manual_extruder_selector();

// signals from interrupt to main loop
extern bool MMU2SLoading;
extern bool inErrorState;
extern int numSlots;
void fixTheProblem(bool showPrevious = false);
void fixIdlCrash(void);

extern long startWakeTime;

typedef enum eFault
{
    FAULT_IDLER_INIT_0, FAULT_IDLER_INIT_1, FAULT_IDLER_INIT_2,
    FAULT_PULLEY_INIT_0, FAULT_PULLEY_INIT_1, FAULT_PULLEY_INIT_2,
} Fault;

#endif //_MAIN_H
