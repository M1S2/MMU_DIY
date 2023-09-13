// mmctl.cpp - multimaterial switcher control
#include "main.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "led.h"
#include "spi.h"
#include "mmctl.h"
#include "motion.h"
#include "Buttons.h"
#include "config.h"
#include "uart.h"

// public variables:
int8_t active_extruder = -1;
int8_t previous_extruder = -1;
bool isPrinting = false;
bool isEjected = false;
bool isIdlerEngaged = true;

uint8_t idlerSlotAngles[NUM_SLOTS_MAX] = IDLER_SLOT_ANGLES_DEFAULT;

bool feed_filament(void)
{
    bool _loaded = false;
    if (!isFilamentLoaded()) 
    {
        int _c = 0;
        set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
        engage_filament_pulley(true);
        while (!_loaded) 
        {
            if (moveSmooth(AX_PUL, 4000, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success) 
            {
                _delay_ms(10);
                moveSmooth(AX_PUL, 500, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC);
                moveSmooth(AX_PUL, -500, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC, true);
                _delay_ms(10);
                moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC);
                set_led_state(active_extruder, LED_SLOT_SELECTED, 0);
                _loaded = true;
                break;
            } 
            else 
            {
                if (_c < 1)                      // Two attempt to load then give up
                {
                    _delay_ms(10);
                    moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC);
                    fixTheProblem();
                    set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
                    engage_filament_pulley(true);
                } 
                else 
                {
                    _delay_ms(10);
                    moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC);
                    engage_filament_pulley(false);
                    _loaded = false;
                    break;
                }
                _c++;
            }
        }
        disableStepper();
        engage_filament_pulley(false);
    } 
    else 
    {
        /* Filament is already loaded */
    }
    return _loaded;
}

void toolChange(int new_extruder)
{
    isPrinting = true;

    set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);

    previous_extruder = active_extruder;

    if (previous_extruder == new_extruder) 
    {
        if (!isFilamentLoaded()) 
        {
            startWakeTime = millis();
            set_positions(new_extruder);
            set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
            
            load_filament_withSensor();
        }
    } 
    else 
    {
        if (isFilamentLoaded()) //unload filament if you need to
        {
            unload_filament_withSensor(previous_extruder);
        }
        set_positions(new_extruder, true);
        set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
        _delay_ms(500);
        load_filament_withSensor();
    }
    set_led_state(active_extruder, LED_SLOT_SELECTED, 0);
}

/**
 * @brief Eject Filament
 * move selector sideways and push filament forward little bit, so user can catch it,
 * unpark idler at the end to user can pull filament out
 * @param extruder: extruder channel (0..4)
 */
void eject_filament(uint8_t extruder)
{
    // if there is still filament detected by PINDA unload it first
    if (isFilamentLoaded()) 
    {
        unload_filament_withSensor();
    }

    set_positions(extruder, true);
    isEjected = true;

    engage_filament_pulley(true);

    // push filament forward
    move_pulley(PULLEY_EJECT_STEPS, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]]);

    // unpark idler so user can easily remove filament
    engage_filament_pulley(false);
    disableStepper();
}

void recover_after_eject()
{
    while (isFilamentLoaded())
    {
        fixTheProblem();
    }
    startWakeTime = millis();
    set_positions(active_extruder);     // Return to previous active extruder
    isEjected = false;
}

void load_filament_withSensor(uint16_t setupBowLen)
{
    uint8_t retries = 1;
    bool _retry = true;
    do 
    {
        engage_filament_pulley(true); // get in contact with filament

        // load filament until FINDA senses end of the filament, means correctly loaded into the selector
        // we can expect something like 570 steps to get in sensor, try 1000 incase user is feeding to pulley
        if (moveSmooth(AX_PUL, 2000, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success) // Move to Pulley
        {
            txFINDAStatus();
            if (setupBowLen != 0) 
            {
                moveSmooth(AX_PUL, setupBowLen, filament_lookup_table[IDX_FIL_TABLE_MAX_SPEED_PUL][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_ACC_FEED_PUL][filament_type[active_extruder]]); // Load filament down to MK3-FSensor
                _retry = false;
            } 
            else 
            {
                moveSmooth(AX_PUL, 1000, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]]); // Go 1000 steps more to get past FINDA before ramping.
                moveSmooth(AX_PUL, BOWDEN_LENGTH - 1000, filament_lookup_table[IDX_FIL_TABLE_MAX_SPEED_PUL][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_ACC_FEED_PUL][filament_type[active_extruder]]);      // Load filament down to near MK3-FSensor
                IR_SENSOR   = false;
                if (moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FSENSOR_SENSE_STEPS][filament_type[active_extruder]], 200, GLOBAL_ACC, false, true) == MR_Success) 
                {
                    set_led_state(active_extruder, LED_SLOT_SELECTED, 0);
                    _retry = false;
                }
                else
                {
                    // Loading failed @ IR_SENSOR
                    fixTheProblem();
                    set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
                }
            }
        }
        else
        {
            if (retries > 0)
            { 
                setIDL2pos(active_extruder);
                retries--;
            }
            else
            {
                // Loading failed @ FINDA
                fixTheProblem();
                set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
            }
        }
    } while (_retry);
    startWakeTime = millis();  // Start/Reset wakeTimer
}

/**
 * @brief unload_filament_withSensor
 * unloads filament from extruder - filament is above Bondtech gears
 */
void unload_filament_withSensor(uint8_t extruder)
{
    int unloadFINDACheckSteps = -3000;
    engage_filament_pulley(true); // get in contact with filament
    _delay_ms(40);
    moveSmooth(AX_PUL, -(30*AX_PUL_STEP_MM_Ratio), filament_lookup_table[IDX_FIL_TABLE_UNLOADSPEED][filament_type[extruder]], GLOBAL_ACC);

    if (isFilamentLoaded()) 
    {
        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH -(20*AX_PUL_STEP_MM_Ratio)) * -1), filament_lookup_table[IDX_FIL_TABLE_MAX_SPEED_PUL][filament_type[extruder]], filament_lookup_table[IDX_FIL_TABLE_ACC_FEED_PUL][filament_type[extruder]], true) == MR_Success)
        { 
            goto loop;
        }
        if (filament_type[extruder] == 1)
        {
            unloadFINDACheckSteps = -5000;
        }
        if (moveSmooth(AX_PUL, unloadFINDACheckSteps, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
        {
            loop:
            moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[extruder]], GLOBAL_ACC); // move to filament parking position
        } 
        else if (isFilamentLoaded()) 
        {
            // Unloading failed to MK3
            if (extruder != active_extruder) 
            {
                fixTheProblem(true);
            }
            else
            {
                fixTheProblem();
            }
            set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
        }
    }
    else
    {
        // Unloading failed to MK3
        if (extruder != active_extruder)
        {
            fixTheProblem(true);
        }
        else
        {
            fixTheProblem();
        }
        set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
    }
    disableStepper();
}

/**
 * @brief unload_filament_withSensor
 * unloads filament from extruder - filament is above Bondtech gears
 */
void unload_filament_forSetup(uint16_t distance, uint8_t extruder)
{
    int unloadFINDACheckSteps = -3000;
    if (isFilamentLoaded()) 
    { 
        engage_filament_pulley(true); // get in contact with filament
        _delay_ms(40);
        if (moveSmooth(AX_PUL, (distance * -1), filament_lookup_table[IDX_FIL_TABLE_MAX_SPEED_PUL][filament_type[extruder]], filament_lookup_table[IDX_FIL_TABLE_ACC_FEED_PUL][filament_type[extruder]], true) == MR_Success)
        {
            goto loop;
        }
        if (filament_type[extruder] == 1)
        {
            unloadFINDACheckSteps = -5000;
        }
        if (moveSmooth(AX_PUL, unloadFINDACheckSteps, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
        {
            loop:
            moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[extruder]], GLOBAL_ACC);   // move to filament parking position
        } 
        else if (isFilamentLoaded()) 
        {
            // Unloading failed to MK3
            if (extruder != active_extruder)
            {
                fixTheProblem(true);
            }
            else
            {
                fixTheProblem();
            }
            set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
        }
    }
    
    disableStepper();
    engage_filament_pulley(false);
}

/**
 * @brief load_filament_intoExtruder
 * loads filament after confirmed by printer into the Bontech
 * pulley gears so they can grab them.
 * We reduce here stepwise the motor current, to prevent grinding into the
 * filament as good as possible.
 *
 * TODO 1: this procedure is most important for high reliability.
 * The speed must be set accordingly to the settings in the slicer
 */
void load_filament_into_extruder()
{
    set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE, 0);
    engage_filament_pulley(true); // get in contact with filament

    move_pulley(150, filament_lookup_table[IDX_FIL_TABLE_L2EXSTAGEONE][filament_type[active_extruder]]);
    move_pulley(170, filament_lookup_table[IDX_FIL_TABLE_L2EXSTAGEONE][filament_type[active_extruder]]);
    move_pulley(820, filament_lookup_table[IDX_FIL_TABLE_L2EXSTAGETWO][filament_type[active_extruder]]);
    disableStepper();
    engage_filament_pulley(false); // release contact with filament
    set_led_state(active_extruder, LED_SLOT_SELECTED, 0);
}

/**
 * @brief engage_filament_pulley
 * Turns the idler drum to engage or disengage the filament pully
 * @param engage
 * If true, pully can drive the filament afterwards
 * if false, idler will be parked, so the filament can move freely
 */
void engage_filament_pulley(bool engage)
{
    if (!isIdlerEngaged && engage)  // get idler in contact with filament
    {
        isIdlerEngaged = true;
    } 
    else if (isIdlerEngaged && !engage)  // park idler so filament can move freely
    {
        isIdlerEngaged = false;
    }
    setIDL2pos(active_extruder);
    _delay_ms(500);
}

void set_positions(uint8_t _next_extruder, bool update_extruders)
{
    if (update_extruders) 
    {
        previous_extruder = active_extruder;
        FilamentLoaded::set(active_extruder);  
    }
    setIDL2pos(_next_extruder);
}

void setIDL2pos(uint8_t _next_extruder)
{
    if (_next_extruder >= numSlots)
    {
        _next_extruder = numSlots - 1;
    }

    if(isIdlerEngaged)
    {
        move_idler(idlerSlotAngles[_next_extruder]);
    }
    else
    {
        if(_next_extruder < numSlots - 2)
        {
            move_idler(idlerSlotAngles[_next_extruder + 1]);
        }
        else
        {
            move_idler(idlerSlotAngles[_next_extruder - 1]);
        }
    }

    active_extruder = _next_extruder;
}
