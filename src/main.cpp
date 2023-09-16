//! @file

#include "main.h"

// public variables:
bool MMU2SLoading = false;
bool m600RunoutChanging = false;
bool inErrorState = false;
long startWakeTime;
int numSlots;
void process_commands(void);

//! LED indication of states
//!
//! Slot0   | Slot1   | Slot2   | Slot3   | Slot4   | meaning
//! ------- | ------- | ------- | ------- | ------- | -----------------------
//! on/off  | on/off  | on/off  | on/off  | on/off  | Show the number of slots (number of on LEDs)
//! blink   | 0       | 0       | 0       | 0       | EEPROM initialized
//! 0       | blink   | 0       | 0       | 0       | UART initialized
//! 0       | 0       | blink   | 0       | 0       | Idler/Stepper initialized
//!
void setup()
{
    // Setup all button pins to be inputs and enable the pullups, enable the button timer
    pinMode(PIN_BTN_LEFT, INPUT);
    pinMode(PIN_BTN_MIDDLE, INPUT);
    pinMode(PIN_BTN_RIGHT, INPUT);
    pullup(PIN_BTN_LEFT);
    pullup(PIN_BTN_MIDDLE);
    pullup(PIN_BTN_RIGHT);
    initButtonTimer();

    pinMode(PIN_LED_DEBUG, OUTPUT);
    digitalWrite(PIN_LED_DEBUG, HIGH);

    LEDS.setOutput(PIN_LED_DIN);
    PORTA |= 0x02;  // Set Button ADC Pin High
    servoIdler.attach(PIN_IDL_SERVO);

    // Detect the number of connected slot PCBs and show it
    numSlots = detect_numSlots();
    for(int i = 0; i < numSlots; i++)
    {
        set_led(i, COLOR_BLUE, false);
    }
    _delay_ms(1000);
    clr_leds();

    DDRC |= (1 << PC4) | (1 << PC5) | (1 << PC6) | (1 << PC7);      // Set stepper and servo pins as outputs

    permanentStorageInit();
    // Load the active_extruder from the EEPROM
    uint8_t filament = 0;
    FilamentLoaded::get(filament);
    active_extruder = filament;
    if(active_extruder == numSlots) { active_extruder = numSlots - 1; }
    startWakeTime = millis();
    led_blink(0);
    
    sei();
    initUart();
    led_blink(1);

    set_positions(active_extruder);     // Move to previous active extruder
    disableStepper();
    led_blink(2);

    sendStringToPrinter((char*)"start");

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
    do 
    {
        set_led_state(0, LED_DELETE_EEPROM_MENU);

        if(get_key_short(1 << KEY_LEFT) || get_key_long(1 << KEY_LEFT) || get_key_short(1 << KEY_RIGHT) || get_key_long(1 << KEY_RIGHT) || get_key_short(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
        else if (get_key_long(1 << KEY_MIDDLE))
        {
            // Delete EEPROM
            #warning Implement EEPROM delete function
            set_led_state(0, LED_DELETE_EEPROM_FINISHED);
            _exit = true;
        }

#warning Implement exit from EEPROM delete menu after Timeout

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

        if (!isFilamentLoaded()) 
        {
            engage_filament_pulley(true);
            baseMenu();
        } 
        else 
        {
            /* no manual extruder selection supported when filament is loaded */
        }
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

    if (((millis() - startWakeTime) > WAKE_TIMER) && !isFilamentLoaded() && !isPrinting)
    {
        disableStepper();
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
    bool previouslyEngaged = isIdlerEngaged;

    engage_filament_pulley(false);                    // park the idler stepper motor

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

void fixIdlCrash(void) 
{
    inErrorState = true;

    while (!get_key_short(1 << KEY_MIDDLE)) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        if (isFilamentLoaded()) 
        {
            set_led_state(active_extruder, LED_SLOT_ERROR_FILAMENT_PRESENT);
        } 
        else
        {
            set_led_state(active_extruder, LED_SLOT_ERROR_NO_FILAMENT);
        }
    }
    inErrorState = false;
    setIDL2pos(active_extruder);
}
