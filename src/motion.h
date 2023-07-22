// motion.h

#ifndef _MOTION_h
#define _MOTION_h

//#include <inttypes.h>
//#include <stdbool.h>
#include "permanent_storage.h"
#include "config.h"
#include "mmctl.h"

extern int8_t filament_type[EXTRUDERS];
extern int filament_lookup_table[9][3]; // [X][Y]Two-dimensional Array of extruder and used variables
extern const uint8_t IDLER_PARKING_STEPS;
extern const uint16_t EJECT_PULLEY_STEPS;
extern const int idlerStepPositionsFromHome[EXTRUDERS+1];
extern uint16_t BOWDEN_LENGTH;
extern uint8_t idlSGFailCount;
extern BowdenLength bowdenLength;
extern uint16_t MAX_SPEED_IDLER;
enum MotReturn
{
    MR_Success,
    MR_Failed
};

bool move_idler(int mm, uint16_t speed = MAX_SPEED_IDLER);
void move_pulley(int mm, uint16_t speed = filament_lookup_table[0][0]);
void enableAllSteppers(void);
void disableAllSteppers(void);
void enableStepper(int axis);
void disableStepper(int axis);
MotReturn homeIdlerSmooth(bool toLastFilament = false);

MotReturn moveSmooth(uint8_t axis, int steps, int speed, float acc = GLOBAL_ACC, bool withFindaDetection = false, bool withIR_SENSORDetection = false);
#endif
