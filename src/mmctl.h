// mmctl.h - multimaterial switcher control
#ifndef _MMCTL_H
#define _MMCTL_H

#include <inttypes.h>
#include "config.h"

// public variables:
extern bool isPrinting;
extern int8_t active_extruder;
extern int8_t previous_extruder;
extern bool isEjected;
extern bool isIdlerEngaged;
extern uint8_t idlerSlotAngles[NUM_SLOTS_MAX];

// functions:
void toolChange(int new_extruder);
bool feed_filament(void);
void engage_filament_pulley(bool engage);
void load_filament_withSensor(uint16_t setupBowLen = 0);
void unload_filament_withSensor(uint8_t extruder = active_extruder);
void unload_filament_forSetup(uint16_t distance, uint8_t extruder = active_extruder);
void load_filament_into_extruder();
void set_positions(uint8_t _next_extruder, bool update_extruders = false);
void setIDL2pos(uint8_t _next_extruder);
void eject_filament(uint8_t extruder);
void recover_after_eject();

#endif //_MMCTL_H
