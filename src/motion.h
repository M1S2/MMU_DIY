// motion.h

#ifndef _MOTION_h
#define _MOTION_h

#include "permanent_storage.h"
#include "config.h"
#include "mmctl.h"
#include "Servo/src/Servo.h"

#define IDX_FIL_TABLE_MAX_SPEED_PUL             0
#define IDX_FIL_TABLE_ACC_FEED_PUL              1
#define IDX_FIL_TABLE_FILAMENT_PARKING_STEPS    2
#define IDX_FIL_TABLE_FSENSOR_SENSE_STEPS       3
#define IDX_FIL_TABLE_FEED_SPEED_PUL            4
#define IDX_FIL_TABLE_L2EXSTAGEONE              5
#define IDX_FIL_TABLE_L2EXSTAGETWO              6
#define IDX_FIL_TABLE_UNLOADSPEED               7

extern int8_t filament_type[NUM_SLOTS_MAX];
extern int filament_lookup_table[IDX_FIL_TABLE_UNLOADSPEED + 1][3]; // [X][Y]Two-dimensional Array of extruder and used variables
extern uint16_t BOWDEN_LENGTH;
extern BowdenLength bowdenLength;

extern Servo servoIdler;

enum MotReturn
{
    MR_Success,
    MR_Failed
};

void move_idler(int deltaAngle);
void move_pulley(int steps, uint16_t speed = filament_lookup_table[IDX_FIL_TABLE_MAX_SPEED_PUL][0]);
void enableAllSteppers(void);
void disableAllSteppers(void);
void enableStepper(int axis);
void disableStepper(int axis);
void parkIdler();

MotReturn moveSmooth(uint8_t axis, int steps, int speed, float acc = GLOBAL_ACC, bool withFindaDetection = false, bool withIR_SENSORDetection = false);
#endif
