// motion.h

#ifndef _MOTION_h
#define _MOTION_h

#include "permanent_storage.h"
#include "config.h"
#include "mmctl.h"
#include "Servo.h"

extern int8_t filament_type[NUM_SLOTS_MAX];
extern int filament_lookup_table[9][3]; // [X][Y]Two-dimensional Array of extruder and used variables
extern uint16_t BOWDEN_LENGTH;
extern BowdenLength bowdenLength;

extern Servo servoIdler;

enum MotReturn
{
    MR_Success,
    MR_Failed
};

void move_idler(int deltaAngle);
void move_pulley(int steps, uint16_t speed = filament_lookup_table[0][0]);
void enableAllSteppers(void);
void disableAllSteppers(void);
void enableStepper(int axis);
void disableStepper(int axis);
void homeIdler(bool toLastFilament = false);

MotReturn moveSmooth(uint8_t axis, int steps, int speed, float acc = GLOBAL_ACC, bool withFindaDetection = false, bool withIR_SENSORDetection = false);
#endif
