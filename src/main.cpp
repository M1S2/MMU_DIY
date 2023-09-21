//! @file

#include "main.h"

// public variables:
bool inErrorState = false;
long startWakeTime;
int numSlots;
void process_commands(void);

/// @brief Setup all necessary pins and parts of the hardware
void setup()
{
    // init debug LED pin as output and activate LED during init
    pinMode(PIN_LED_DEBUG, OUTPUT);
    digitalWrite(PIN_LED_DEBUG, HIGH);

    // init button part
    initButtonPins();
    initButtonTimer();

    // init UART
    sei();
    initUart();

    LEDS.setOutput(PIN_LED_DIN);
    // Detect the number of connected slot PCBs and show it
    numSlots = detect_numSlots();
    for(int i = 0; i < numSlots; i++)
    {
        set_led(i, COLOR_BLUE, false);
    }
    _delay_ms(1000);
    clr_leds();

    // Init the EEPROM and load the active_extruder from the EEPROM
    permanentStorageInit();
    uint8_t filament = 0;
    FilamentLoaded::get(filament);
    active_extruder = filament;
    if(active_extruder == numSlots) { active_extruder = numSlots - 1; }
    startWakeTime = millis();

    // init servo and stepper part
    DDRC |= (1 << PC4) | (1 << PC5) | (1 << PC6) | (1 << PC7);      // Set stepper and servo pins as outputs
    servoIdler.attach(PIN_IDL_SERVO);
    set_positions(active_extruder);                                 // Move to previous active extruder
    disableStepper();

    // send "start" string to the printer to signal that the MMU is alive
    sendStringToPrinter((char*)"start");

    // turn off debug LED at end of setup part
    digitalWrite(PIN_LED_DEBUG, LOW);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | return to base menu
//! RIGHT           | return to base menu
//! MIDDLE          | return to base menu
//! MIDDLE (long)   | delete EEPROM and return to base menu
//! Timeout         | return to base menu
void eepromDeleteMenu()
{
    bool _exit = false;
    long enterEepromDeleteMenuTime = millis();
    do 
    {
        set_led_state(0, LED_DELETE_EEPROM_MENU);

        if(get_key_short(1 << KEY_LEFT) || get_key_long(1 << KEY_LEFT) || get_key_short(1 << KEY_RIGHT) || get_key_long(1 << KEY_RIGHT) || get_key_short(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
        else if (((millis() - enterEepromDeleteMenuTime) > 10000))   // The user has 10 seconds time to delete the EEPROM after entering the menu. After that time the menu is exited automatically and must be reentered.
        {
            _exit = true;
        }
        else if (get_key_long(1 << KEY_MIDDLE))
        {
            // Delete EEPROM
            eepromEraseAll();
            set_led_state(0, LED_DELETE_EEPROM_FINISHED);
            _exit = true;
        }
    } while (!_exit);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Move to previous slot (if not at first slot)
//! RIGHT           | Move to next slot (if not at last slot)
//! MIDDLE          | Feed filament
//! MIDDLE (long)   | Enter slot setup menu for the last selected slot
//! RIGHT & LEFT    | Enter EEPROM deletion menu
void baseMenu()
{
    uint8_t keyStateShortRight = get_key_short(1 << KEY_RIGHT);
    uint8_t keyStateShortLeft = get_key_short(1 << KEY_LEFT);

    if(keyStateShortLeft && keyStateShortRight)
    {
        eepromDeleteMenu();
        set_led_state(active_extruder, LED_SLOT_SELECTED);
    }
    else if(keyStateShortLeft || get_key_rpt(1 << KEY_LEFT))
    {
        // move to previous slot
        if (active_extruder > 0) set_positions(active_extruder - 1, true);
    }
    else if(keyStateShortRight || get_key_rpt(1 << KEY_RIGHT))
    {
        // move to next slot
        if (active_extruder < numSlots) set_positions(active_extruder + 1, true);
    }
    else if(get_key_long(1 << KEY_MIDDLE))
    {
        slotSetupMenu();
        set_led_state(active_extruder, LED_SLOT_SELECTED);
    }
    else if(get_key_short(1 << KEY_MIDDLE))
    {
        feed_filament();
    }
}

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
        set_led_state(active_extruder, LED_SLOT_SELECTED);
        engage_filament_pulley(true);
        baseMenu();
    } 
    else if (isEjected) 
    {
        if(get_key_short(1 << KEY_RIGHT))
        {
            engage_filament_pulley(true);
            moveSmooth(AX_PUL, (PULLEY_EJECT_STEPS * -1), filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]], GLOBAL_ACC);
            engage_filament_pulley(false);
        }
    }

    // Disable the steppers if they were inactive for some time and the filament is not loaded and no print is running
    if (((millis() - startWakeTime) > WAKE_TIMER) && !isFilamentLoaded() && !isPrinting)
    {
        disableStepper();
    }
}

void process_commands(void)
{
    cli();
    // Copy volatile vars as local
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

    if (tData1 == 'T')
    {
        //Tx Tool Change CMD Received
        if ((tData2 - '0') < numSlots) 
        {
            isPrinting = true;
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
                /* Filament is already loaded */
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
        set_led_state(active_extruder, LED_SLOT_OPERATION_ACTIVE);
        // Ux Unload filament CMD Received
        unload_filament_withSensor();
        engage_filament_pulley(false);
        set_led_state(active_extruder, LED_SLOT_SELECTED);
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
    bool previouslyEngaged = isIdlerEngaged;

    engage_filament_pulley(false);                    // park the idler 

    inErrorState = true;

    while (!get_key_short(1 << KEY_MIDDLE) || isFilamentLoaded()) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        if (!showPrevious) 
        {
            if(get_key_short(1 << KEY_RIGHT))
            {
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
            }
            else if(get_key_short(1 << KEY_LEFT))
            {
                engage_filament_pulley(true);
                moveSmooth(AX_PUL, 300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8);
                engage_filament_pulley(false);     
            }

            if (isFilamentLoaded()) 
            {
                set_led_state(active_extruder, LED_SLOT_ERROR_FILAMENT_PRESENT);
            } 
            else 
            {
                set_led_state(active_extruder, LED_SLOT_ERROR_NO_FILAMENT);
            }
        } 
        else 
        {
            if(get_key_short(1 << KEY_RIGHT))
            {
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
            }
            else if(get_key_short(1 << KEY_LEFT))
            {
                engage_filament_pulley(true);
                moveSmooth(AX_PUL, 300, filament_lookup_table[IDX_FIL_TABLE_FEED_SPEED_PUL][filament_type[previous_extruder]]*1.8);
                engage_filament_pulley(false);
            }
            _delay_ms(100);
            clr_leds();
            if (active_extruder != previous_extruder) 
            {
                set_led_state(active_extruder, LED_SLOT_SELECTED);
            }
            _delay_ms(100);
            if (isFilamentLoaded()) 
            {
                set_led(previous_extruder, COLOR_RED, active_extruder == previous_extruder);
            } 
            else 
            {
                set_led(previous_extruder, COLOR_GREEN, active_extruder == previous_extruder);
            }
        }
    }
    _delay_ms(100);
    
    inErrorState = false;

    startWakeTime = millis();
    set_positions(active_extruder);     // Return to previous active extruder
    FilamentLoaded::set(active_extruder);

    engage_filament_pulley(previouslyEngaged);
}
