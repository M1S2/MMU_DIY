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
int8_t activeIdlPos = -1;
int8_t previous_extruder = -1;
uint16_t toolChanges = 0;
uint16_t trackToolChanges = 0;
bool isIdlerParked = true;
bool isPrinting = false;
bool isEjected = false;
bool isHomed = false;
bool homedOnUnload = false;

bool feed_filament(void)
{
    bool _loaded = false;
    if (!isHomed && !isFilamentLoaded())
    {
        home(true);
    }
    if (!isFilamentLoaded()) 
    {
        int _c = 0;
        set_led(active_extruder, COLOR_BLUE);
        engage_filament_pulley(true);
        while (!_loaded) 
        {
            if (moveSmooth(AX_PUL, 4000, filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success) 
            {
                delay(10);
                moveSmooth(AX_PUL, 500, filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC);
                moveSmooth(AX_PUL, -500, filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC, true);
                delay(10);
                moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[active_extruder]], filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC);
                set_led(active_extruder, COLOR_GREEN);
                _loaded = true;
                break;
            } 
            else 
            {
                if (_c < 1)                      // Two attempt to load then give up
                {
                    delay(10);
                    moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[active_extruder]], filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC);
                    fixTheProblem();
                    engage_filament_pulley(true);
                } 
                else 
                {
                    delay(10);
                    moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[active_extruder]], filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC);
                    engage_filament_pulley(false);
                    _loaded = false;
                    break;
                }
                _c++;
            }
        }
        disableStepper(AX_PUL);
        engage_filament_pulley(false);
    } 
    else 
    {
        txPayload((char*)"Z1---");
        delay(1500);
        txPayload((char*)"ZZZ--");
    }
    return _loaded;
}

void toolChange(int new_extruder)
{
    isPrinting = true;

    set_led(active_extruder, COLOR_BLUE);

    previous_extruder = active_extruder;
    active_extruder = new_extruder;

    if (previous_extruder == active_extruder) 
    {
        if (!isFilamentLoaded()) 
        {
            if (!isHomed)
            {
                home(true);
            }
            set_led(active_extruder, COLOR_BLUE);
            load_filament_withSensor();
        }
    } 
    else 
    {
        if (isFilamentLoaded()) //unload filament if you need to
        {
            unload_filament_withSensor(previous_extruder);
        }
        if (!isHomed)
        {
            home(true);
        } 
        else if (!homedOnUnload) 
        {
            set_positions(active_extruder, true);
        }
        toolChanges++;
        trackToolChanges++;
        uint8_t toolChangesUpper = (0xFF & (toolChanges >> 8));
        uint8_t toolChangesLower = (0xFF & toolChanges);
        unsigned char txTCU[5] = {'T',toolChangesUpper, toolChangesLower, BLK, BLK};
        txPayload((char*)txTCU);
        set_led(active_extruder, COLOR_BLUE);
        load_filament_withSensor();
        homedOnUnload = false;
    }
    set_led(active_extruder, COLOR_GREEN);
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
#if false
    if (active_extruder == (numSlots - 1))
    {
        setSEL2pos(0);
    }
    else
    {
        setSEL2pos(numSlots);
    }
#endif
    isEjected = true;

    engage_filament_pulley(true);

    // push filament forward
    move_pulley(PULLEY_EJECT_STEPS, filament_lookup_table[5][filament_type[active_extruder]]);

    // unpark idler so user can easily remove filament
    engage_filament_pulley(false);
    disableStepper(AX_PUL);
}

void recover_after_eject()
{
    while (isFilamentLoaded())
    {
        fixTheProblem();
    }
    home(true);
    isEjected = false;
}

void load_filament_withSensor(uint16_t setupBowLen)
{
    uint8_t retries = 1;
    bool _retry = true;
    do 
    {
        if (!isHomed && (setupBowLen == 0))
        {
            home(true);
        }
        engage_filament_pulley(true); // get in contact with filament

        // load filament until FINDA senses end of the filament, means correctly loaded into the selector
        // we can expect something like 570 steps to get in sensor, try 1000 incase user is feeding to pulley
        if (moveSmooth(AX_PUL, 2000, filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success) // Move to Pulley
        {
            txFINDAStatus();
            if (setupBowLen != 0) 
            {
                moveSmooth(AX_PUL, setupBowLen, filament_lookup_table[0][filament_type[active_extruder]], filament_lookup_table[1][filament_type[active_extruder]]); // Load filament down to MK3-FSensor
                _retry = false;
            } 
            else 
            {
                moveSmooth(AX_PUL, 1000, filament_lookup_table[5][filament_type[active_extruder]]); // Go 1000 steps more to get past FINDA before ramping.
                moveSmooth(AX_PUL, BOWDEN_LENGTH - 1000, filament_lookup_table[0][filament_type[active_extruder]], filament_lookup_table[1][filament_type[active_extruder]]);      // Load filament down to near MK3-FSensor
                txPayload((char*)"IRSEN");
                IR_SENSOR   = false;
                if (moveSmooth(AX_PUL, filament_lookup_table[4][filament_type[active_extruder]], 200, GLOBAL_ACC, false, true) == MR_Success) 
                {
                    set_led(active_extruder, COLOR_GREEN);
                    _retry = false;
                }
                else
                {
                    txPayload((char*)"ZL2"); // Report Loading failed @ IR_SENSOR
                    fixTheProblem();
                }
            }
        }
        else
        {
            if (retries > 0)
            { 
                set_idler_toLast_positions(active_extruder);
                retries--;
            }
            else
            {
                txPayload((char*)"ZL1--"); // Report Loading failed @ FINDA
                fixTheProblem();
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
    uint8_t mmPerSecSpeedUpper = (0xFF & ((filament_lookup_table[8][filament_type[extruder]] / AX_PUL_STEP_MM_Ratio) >> 8));
    uint8_t mmPerSecSpeedLower = (0xFF & (filament_lookup_table[8][filament_type[extruder]] / AX_PUL_STEP_MM_Ratio));
    unsigned char txUFR[5] = {'U', mmPerSecSpeedUpper, mmPerSecSpeedLower, BLK, BLK};
    txPayload((char*)txUFR);
    delay(40);
    moveSmooth(AX_PUL, -(30*AX_PUL_STEP_MM_Ratio), filament_lookup_table[8][filament_type[extruder]], GLOBAL_ACC);

    if (isFilamentLoaded()) 
    {
        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH -(20*AX_PUL_STEP_MM_Ratio)) * -1), filament_lookup_table[0][filament_type[extruder]], filament_lookup_table[1][filament_type[extruder]], true) == MR_Success)
        { 
            goto loop;
        }
        if (filament_type[extruder] == 1)
        {
            unloadFINDACheckSteps = -5000;
        }
        if (moveSmooth(AX_PUL, unloadFINDACheckSteps, filament_lookup_table[5][filament_type[extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
        {
            loop:
            moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[extruder]], filament_lookup_table[5][filament_type[extruder]], GLOBAL_ACC); // move to filament parking position
        } 
        else if (isFilamentLoaded()) 
        {
            txPayload((char*)"ZU---"); // Report Unloading failed to MK3
            if (extruder != active_extruder) 
            {
                fixTheProblem(true);
            }
            else
            {
                fixTheProblem();
            }
            homedOnUnload = true;
        }
    }
    else
    {
        txPayload((char*)"ZU---"); // Report Unloading failed to MK3
        if (extruder != active_extruder)
        {
            fixTheProblem(true);
        }
        else
        {
            fixTheProblem();
        }
    }
    disableStepper(AX_PUL);
    engage_filament_pulley(false);
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
        uint8_t mmPerSecSpeedUpper = (0xFF & ((filament_lookup_table[8][filament_type[extruder]] / AX_PUL_STEP_MM_Ratio) >> 8));
        uint8_t mmPerSecSpeedLower = (0xFF & (filament_lookup_table[8][filament_type[extruder]] / AX_PUL_STEP_MM_Ratio));
        unsigned char txUFR[5] = {'U',mmPerSecSpeedUpper, mmPerSecSpeedLower, BLK, BLK};
        txPayload((char*)txUFR);
        delay(40);
        if (moveSmooth(AX_PUL, (distance * -1), filament_lookup_table[0][filament_type[extruder]], filament_lookup_table[1][filament_type[extruder]], true) == MR_Success)
        {
            goto loop;
        }
        if (filament_type[extruder] == 1)
        {
            unloadFINDACheckSteps = -5000;
        }
        if (moveSmooth(AX_PUL, unloadFINDACheckSteps, filament_lookup_table[5][filament_type[extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
        {
            loop:
            moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[extruder]], filament_lookup_table[5][filament_type[extruder]], GLOBAL_ACC);   // move to filament parking position
        } 
        else if (isFilamentLoaded()) 
        {
            txPayload((char*)"ZU---"); // Report Unloading failed to MK3
            if (extruder != active_extruder)
            {
                fixTheProblem(true);
            }
            else
            {
                fixTheProblem();
            }
            homedOnUnload = true;
        }
    }
    
    disableStepper(AX_PUL);
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
    set_led(active_extruder, COLOR_BLUE);
    engage_filament_pulley(true); // get in contact with filament

    move_pulley(150, filament_lookup_table[6][filament_type[active_extruder]]);
    move_pulley(170, filament_lookup_table[6][filament_type[active_extruder]]);
    move_pulley(820, filament_lookup_table[7][filament_type[active_extruder]]);
    disableStepper(AX_PUL);
    engage_filament_pulley(false); // release contact with filament
    set_led(active_extruder, COLOR_GREEN);
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
    if (isIdlerParked && engage)  // get idler in contact with filament
    {
        sendStringToPrinter("engage");
        setIDL2pos(active_extruder);
        isIdlerParked = false;
    } 
    else if (!isIdlerParked && !engage)  // park idler so filament can move freely
    {
        sendStringToPrinter("park");
        parkIdler();
        isIdlerParked = true;
    }
}

#warning home isn't really homing anymore !!!
void home(bool doToolSync)
{
    bool previouslyEngaged = !isIdlerParked;

    /*clr_leds();
    for(int i = 0; i < numSlots; i++)
    {
        set_led(i, COLOR_BLUE);
    }*/

    //homeIdler();

    set_led(active_extruder, COLOR_GREEN);

    isHomed = true;
    startWakeTime = millis();

    if (doToolSync) 
    {
        set_positions(active_extruder);
        FilamentLoaded::set(active_extruder);
        engage_filament_pulley(previouslyEngaged);
        trackToolChanges = 0;
    }
}

void set_positions(uint8_t _next_extruder, bool update_extruders)
{
    if (update_extruders) 
    {
        previous_extruder = active_extruder;
        active_extruder = _next_extruder;
        FilamentLoaded::set(active_extruder);
        unsigned char temp[5] = {'A', 'E', (uint8_t)active_extruder, BLK, BLK};
        txPayload((char*)temp);    
    }
    if (!isHomed)
    {
        home(true);
    }
    else 
    {
        setIDL2pos(_next_extruder);
    }
}

void setIDL2pos(uint8_t _next_extruder)
{
    if (_next_extruder == numSlots)
    {
        _next_extruder -= 1;
    }
    int _idler_steps = (activeIdlPos - _next_extruder) * IDLER_NEXT_FILAMENT_ANGLE;

    move_idler(_idler_steps);
    activeIdlPos = _next_extruder; 
}

void set_idler_toLast_positions(uint8_t _next_extruder)
{
    bool previouslyEngaged = !isIdlerParked;
    //homeIdler();
    setIDL2pos(_next_extruder);
    engage_filament_pulley(previouslyEngaged);
}
