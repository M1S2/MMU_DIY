//! @file

#include "main.h"

// public variables:
bool MMU2SLoading = false;
bool m600RunoutChanging = false;
bool inErrorState = false;
long startWakeTime;
int numSlots;
void process_commands(void);

//! @brief Initialization after reset
//!
//! button | action
//! ------ | ------
//! middle | enter setup
//! right  | continue after error
//!
//! LED indication of states
//!
//! Slot0   | Slot1 | Slot2 | Slot3 | Slot4 | meaning
//! ------- | ----- | ----- | ----- | ----- | -----------------------
//! blink   | 0     | 0     | 0     | 0     | EEPROM initialized
//! 0       | blink | 0     | 0     | 0     | UART initialized
//! 0       | 0     | blink | 0     | 0     | Idler initialized
//!
void setup()
{
    #warning Add logic to detect the number of connected LEDs here
    numSlots = NUM_SLOTS_DEFAULT;
    LEDS.setOutput(PIN_LED_DIN);
    PORTA |= 0x02;  // Set Button ADC Pin High
    servoIdler.attach(PIN_IDL_SERVO);

    permanentStorageInit();
    //Load the active_extruder from the EEPROM
    uint8_t filament = 0;
    FilamentLoaded::get(filament);
    active_extruder = filament;
    if(active_extruder == numSlots) { active_extruder = numSlots - 1; }
    startWakeTime = millis();
    led_blink(0);
    
    sei();
    initUart();
    led_blink(1);

    parkIdler();
    delay(1000);
    set_positions(active_extruder);     // Move to previous active extruder
    led_blink(2);

    if (active_extruder != numSlots) txPayload((char*)"STR--");
    sendStringToPrinter((char*)"start");
}

//! @brief Select filament menu
//!
//! Select filament by pushing left and right button, park position can be also selected.
//!
//! button | action
//! ------ | ------
//! left   | select previous filament
//! right  | select next filament
//!
//! LED indication of states
//!
//! RG | RG | RG | RG | RG | meaning
//! -- | -- | -- | -- | -- | ------------------------
//! 01 | 00 | 00 | 00 | 00 | filament 1
//! 00 | 01 | 00 | 00 | 00 | filament 2
//! 00 | 00 | 01 | 00 | 00 | filament 3
//! 00 | 00 | 00 | 01 | 00 | filament 4
//! 00 | 00 | 00 | 00 | 01 | filament 5
//! 00 | 00 | 00 | 00 | bb | park position
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//! @n b - blinking
void manual_extruder_selector()
{
    if (isIdlerParked) 
    {
        set_led(numSlots - 1, COLOR_BLUE);
        delay(100);
        clr_leds();
    }
    else
    {
        set_led(active_extruder, COLOR_GREEN);
        delay(100);
    }
    delay(200);

    if (!isFilamentLoaded()) 
    {
        switch (buttonClicked()) 
        {
        case BTN_RIGHT:
            if (active_extruder < numSlots) set_positions(active_extruder + 1, true);
            else if (isIdlerParked) txPayload((char*)"X1---");
            break;
        case BTN_LEFT:
            if (isIdlerParked) set_positions(numSlots - 1);     // unpark idler
            else if (active_extruder > 0) set_positions(active_extruder - 1, true);
            break;
        default:
            break;
        }
    } 
    else 
    {
        switch (buttonClicked()) 
        {
          case BTN_RIGHT:
          case BTN_LEFT:
            txPayload((char*)"Z1---");
            delay(1000);
            txPayload((char*)"ZZZ--");
            break;
          default:
            break;
        }
    }
}

int i=0;
//! @brief main loop
//!
//! It is possible to manually select filament and feed it when not printing.
//!
//! button | action
//! ------ | ------
//! middle | feed filament
//!
//! @copydoc manual_extruder_selector()
void loop()
{
    process_commands();    

    if (!isPrinting && !isEjected) 
    {
        manual_extruder_selector();
        if (BTN_MIDDLE == buttonClicked()) 
        {
            if (!isIdlerParked)
            {
                feed_filament();
            }
            else 
            {
                txPayload((char*)"SETUP");
                setupMenu();
            }
        }
    } 
    else if (isEjected) 
    {
        switch (buttonClicked()) 
        {
        case BTN_RIGHT:
            engage_filament_pulley(true);
            moveSmooth(AX_PUL, (PULLEY_EJECT_STEPS * -1), filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]], GLOBAL_ACC);
            engage_filament_pulley(false);
            break;
        default:
            break;
        }
    }

    if (((millis() - startWakeTime) > WAKE_TIMER) && !isFilamentLoaded() && !isPrinting)
    {
        disableAllSteppers();
    }
}

void process_commands(void)
{
    cli();
    // Copy volitale vars as local
    unsigned char tData1 = rxData1;
    unsigned char tData2 = rxData2;
    unsigned char tData3 = rxData3;
    bool confPayload = confirmedPayload;
    if (confPayload) 
    {
        confirmedPayload = false;
    }
    else 
    { 
        tData1 = ' '; tData2 = ' '; tData3 = ' ';
    } 
    sei();
    if (inErrorState) return;

#warning TESTCODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(tData1 == 'I')
    {
        engage_filament_pulley(false);    
    }
    else if(tData1 == 'B')
    {
        engage_filament_pulley(true);
    }
    else if(tData1 == 'Z')
    {
        active_extruder = tData2 - '0';
    }
    else if(tData1 == 'Y')
    {
        if((tData2 - '0') < numSlots)
        {
            set_positions(tData2 - '0');
        }
    }
    // #warning TESTCODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    if (tData1 == 'T')
    {
        //Tx Tool Change CMD Received
        if ((tData2 - '0') < numSlots) 
        {
            m600RunoutChanging = false;
            MMU2SLoading = true;
            toolChange(tData2 - '0');
            sendStringToPrinter(OK);
        }
    } 
    else if (tData1 == 'L') 
    {
        // Lx Load Filament CMD Received
        if ((tData2 - '0') < numSlots) 
        {
            if (isFilamentLoaded()) 
            {
                txPayload((char*)"Z1---");
                delay(1500);
                txPayload((char*)"ZZZ--");
            } 
            else 
            {
                set_positions(tData2 - '0', true);
                feed_filament(); // returns OK and active_extruder to update MK3
            }
            sendStringToPrinter(OK);
        }
    } 
    else if ((tData1 == 'U') && (tData2 == '0')) 
    {
        set_led(active_extruder, COLOR_BLUE);
        // Ux Unload filament CMD Received
        unload_filament_withSensor();
        set_led(active_extruder, COLOR_GREEN);
        sendStringToPrinter(OK);
        isPrinting = false;
    } 
    else if (tData1 == 'S') 
    {
        // Sx Starting CMD Received
        if (tData2 == '0') 
        {
            sendStringToPrinter(OK);
        } 
        else if (tData2 == '1') 
        {
            char tempS1[10];
            sprintf(tempS1, "%hu%s", FW_VERSION, OK);
            sendStringToPrinter(tempS1);
        } 
        else if (tData2 == '2') 
        {
            char tempS2[10];
            sprintf(tempS2, "%hu%s", FW_BUILDNR, OK);
            sendStringToPrinter(tempS2);
        } 
        else if (tData2 == '3') 
        {
            char tempS3[10];
            sprintf(tempS3, "%hu%s", active_extruder, OK);
            sendStringToPrinter(tempS3);
        }
    } 
    else if (tData1 == 'F') 
    {
        // Fxy Filament Type Set CMD Received
        if (((tData2 - '0') < numSlots) && ((tData3 - '0') < 3)) 
        {
            filament_type[tData2 - '0'] = tData3 - '0';
            sendStringToPrinter(OK);
        }
    } 
    else if ((tData1 == 'X') && (tData2 == '0')) 
    {
        // Xx RESET CMD Received (Done by starting program again)
        asm("jmp 0");
    } 
    else if ((tData1 == 'P') && (tData2 == '0')) 
    {
        txFINDAStatus();
    } 
    else if ((tData1 == 'C') && (tData2 == '0')) 
    {
        sendStringToPrinter(OK);
        load_filament_into_extruder();
    } 
    else if  (tData1 == 'E') 
    {
        // Ex Eject Filament X CMD Received
        if ((tData2 - '0') < numSlots) // Ex: eject filament
        {
            m600RunoutChanging = true;
            eject_filament(tData2 - '0');
            sendStringToPrinter(OK);
        }
    } 
    else if ((tData1 == 'R') && (tData2 == '0')) 
    {
        // Rx Recover Post-Eject Filament X CMD Received
        recover_after_eject();
        sendStringToPrinter(OK);
    }
}

//****************************************************************************************************
//* this routine is the common routine called for fixing the filament issues (loading or unloading)
//****************************************************************************************************
void fixTheProblem(bool showPrevious) 
{
    bool previouslyEngaged = !isIdlerParked;
    engage_filament_pulley(false);                    // park the idler stepper motor

    inErrorState = true;

    while ((BTN_MIDDLE != buttonClicked()) || isFilamentLoaded()) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        if (!showPrevious) 
        {
            switch (buttonClicked()) 
            {
                case BTN_RIGHT:
                    engage_filament_pulley(true);
                    if (isFilamentLoaded()) 
                    {
                        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH * 1.5) * -1), filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
                        {
                            moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[active_extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]], GLOBAL_ACC); // move to filament parking position
                        }
                    } 
                    else 
                    {
                        moveSmooth(AX_PUL, -300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[active_extruder]]);
                    }
                    engage_filament_pulley(false);                 
                    disableStepper(AX_IDL);
                    break;
                case BTN_LEFT:
                    engage_filament_pulley(true);
                    moveSmooth(AX_PUL, 300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8);
                    engage_filament_pulley(false);                   
                    disableStepper(AX_IDL);
                    break;
                default:
                    break;
            }
            delay(100);
            clr_leds();
            delay(100);
            if (isFilamentLoaded()) 
            {
                set_led(active_extruder, COLOR_RED);
            } 
            else 
            {
                set_led(active_extruder, COLOR_GREEN);
            }
        } 
        else 
        {
            switch (buttonClicked()) 
            {
                case BTN_RIGHT:
                    engage_filament_pulley(true);
                    if (isFilamentLoaded()) 
                    {
                        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH * 1.5) * -1), filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8, GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
                        {
                            moveSmooth(AX_PUL, filament_lookup_table[IDX_FIL_TABLE_FILAMENT_PARKING_STEPS][filament_type[previous_extruder]], filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8, GLOBAL_ACC); // move to filament parking position
                        }
                    } 
                    else 
                    {
                        moveSmooth(AX_PUL, -300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8);
                    }
                    engage_filament_pulley(false);
                    disableStepper(AX_IDL);
                    break;
                case BTN_LEFT:
                    engage_filament_pulley(true);
                    moveSmooth(AX_PUL, 300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8);
                    engage_filament_pulley(false);
                    disableStepper(AX_IDL);
                    break;
                default:
                    break;
            }
            delay(100);
            clr_leds();
            if (active_extruder != previous_extruder) 
            {
                set_led(active_extruder, COLOR_GREEN);
            }
            delay(100);
            if (isFilamentLoaded()) 
            {
                set_led(previous_extruder, COLOR_RED);
            } 
            else 
            {
                set_led(previous_extruder, COLOR_GREEN);
            }
        }
    }
    delay(100);
    
    inErrorState = false;
    txPayload((char*)"ZZZ--"); // Clear MK3 Message

    startWakeTime = millis();
    set_positions(active_extruder);     // Return to previous active extruder
    FilamentLoaded::set(active_extruder);
    engage_filament_pulley(previouslyEngaged);
}

void fixIdlCrash(void) 
{
    disableStepper(AX_IDL);                            // turn OFF the idler stepper motor
    inErrorState = true;

    while (BTN_MIDDLE != buttonClicked()) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        delay(100);
        clr_leds();
        delay(100);
        if (isFilamentLoaded()) 
        {
            set_led(active_extruder, COLOR_RED);
        } 
        else set_led(active_extruder, COLOR_GREEN);
    }
    inErrorState = false;
    set_idler_toLast_positions(active_extruder);
}
