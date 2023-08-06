#include "motion.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <Arduino.h>
#include "main.h"
#include "uart.h"
#include "Buttons.h"

// public variables:
BowdenLength bowdenLength;
uint16_t BOWDEN_LENGTH = bowdenLength.get();
int8_t filament_type[NUM_SLOTS_MAX] = { 0, 0, 0, 0, 0};
int filament_lookup_table[9][3] =
{{TYPE_0_MAX_SPPED_PUL,               TYPE_1_MAX_SPPED_PUL,               TYPE_2_MAX_SPPED_PUL},
 {TYPE_0_ACC_FEED_PUL,                TYPE_1_ACC_FEED_PUL,                TYPE_2_ACC_FEED_PUL},
 {0,                                  0,                                  0},  // Not used with IR_SENSOR
 {TYPE_0_FILAMENT_PARKING_STEPS,      TYPE_1_FILAMENT_PARKING_STEPS,      TYPE_2_FILAMENT_PARKING_STEPS},
 {TYPE_0_FSensor_Sense_STEPS,         TYPE_1_FSensor_Sense_STEPS,         TYPE_2_FSensor_Sense_STEPS},
 {TYPE_0_FEED_SPEED_PUL,              TYPE_1_FEED_SPEED_PUL,              TYPE_2_FEED_SPEED_PUL},
 {TYPE_0_L2ExStageOne,                TYPE_1_L2ExStageOne,                TYPE_2_L2ExStageOne},
 {TYPE_0_L2ExStageTwo,                TYPE_1_L2ExStageTwo,                TYPE_2_L2ExStageTwo},
 {TYPE_0_UnloadSpeed,                 TYPE_1_UnloadSpeed,                 TYPE_2_UnloadSpeed}
};

Servo servoIdler;
int currentServoAngle = 0;

// private functions:
static uint16_t set_pulley_direction(int steps);

/**
 * @brief move_idler
 * @param deltaAngle, delta angle by which the servo is rotated
 */
void move_idler(int deltaAngle)
{
    currentServoAngle += deltaAngle;
    servoIdler.write(currentServoAngle);
}

void move_pulley(int steps, uint16_t speed)
{
    moveSmooth(AX_PUL, steps, speed);
}

/**
 * @brief set_pulley_direction
 * @param steps, positive (push) or negative (pull)
 * @return abs(steps)
 */
uint16_t set_pulley_direction(int steps)
{
    if (steps < 0)
    {
        steps = steps * -1;
        PIN_PUL_DIR_HIGH;
    }
    else
    {
        PIN_PUL_DIR_LOW;
    }
    return steps;
}

void enableAllSteppers(void)
{
    PIN_PUL_EN_LOW;
}

void disableAllSteppers(void)
{
    PIN_PUL_EN_HIGH;
    isHomed = false;
}

void enableStepper(int axis)
{
    switch (axis) 
    {
    case AX_PUL:
        PIN_PUL_DIR_LOW;
        break;
    case AX_IDL:
        break;
    }
}

void disableStepper(int axis)
{
    switch (axis) 
    {
    case AX_PUL:
        PIN_PUL_DIR_HIGH;
        break;
    case AX_IDL:
        break;
    }
}

void homeIdler(bool toLastFilament)
{
    currentServoAngle = IDLER_HOME_ANGLE_ABSOLUTE;
    move_idler(0);

    isIdlerParked = false;
    activeIdlPos = numSlots;
    if (toLastFilament) 
    {
        uint8_t filament = 0;
        FilamentLoaded::get(filament);
        active_extruder = filament;
        setIDL2pos(active_extruder);
        engage_filament_pulley(false);
    }
}

/**
 * @brief moveTest
 * @param axis, index of axis, use AX_PUL
 * @param steps, number of micro steps to move
 * @param speed, max. speed
 * @return
 */
MotReturn moveSmooth(uint8_t axis, int steps, int speed, float acc, bool withFindaDetection, bool withIR_SENSORDetection)
{
    enableStepper(axis);
    startWakeTime = millis();
    MotReturn ret = MR_Success;
    if (withFindaDetection or withIR_SENSORDetection) ret = MR_Failed;
    float vMax = speed;
    float v0 = 200; // steps/s, minimum speed
    float v = v0; // current speed
    int accSteps = 0; // number of steps for acceleration
    int stepsDone = 0;
    int stepsLeft = 0;

    switch (axis) 
    {
        case AX_PUL:
            stepsLeft = set_pulley_direction(steps);
            break;
    }

    enum State 
    {
        Accelerate = 0,
        ConstVelocity = 1,
        Decelerate = 2,
    };

    State st = Accelerate;

    while (stepsLeft) 
    {
        switch (axis) 
        {
        case AX_PUL:
            PIN_PUL_STP_HIGH;
            PIN_PUL_STP_LOW;
            if (withFindaDetection && ( steps > 0 ) && isFilamentLoaded()) 
            {
              return MR_Success;
            }
            if (withFindaDetection && ( steps < 0 ) && (isFilamentLoaded() == false)) 
            {
              return MR_Success;
            }
            if (withIR_SENSORDetection && IR_SENSOR) 
            {
                IR_SENSOR = false;
                return MR_Success;
            }
            break;
        }

        stepsDone++;
        stepsLeft--;

        float dt = 1 / v;
        delayMicroseconds(1e6 * dt);

        switch (st) 
        {
        case Accelerate:
            v += acc * dt;
            if (v >= vMax) 
            {
                accSteps = stepsDone;
                st = ConstVelocity;

                v = vMax;
            } 
            else if (stepsDone > stepsLeft) 
            {
                accSteps = stepsDone;
                st = Decelerate;
            }
            break;
        case ConstVelocity: 
        {
            if (stepsLeft <= accSteps) 
            {
                st = Decelerate;
            }
        }
        break;
        case Decelerate: 
        {
            v -= acc * dt;
            if (v < v0) 
            {
                v = v0;
            }
        }
        break;
        }
    }
    return ret;
}
