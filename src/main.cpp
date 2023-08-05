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
//! RG | RG | RG | RG | RG | meaning
//! -- | -- | -- | -- | -- | ------------------------
//! 00 | 00 | 00 | 00 | 0b | Shift register initialized
//! 00 | 00 | 00 | 0b | 00 | uart initialized
//! 00 | 00 | 0b | 00 | 00 | spi initialized
//! 00 | 0b | 00 | 00 | 00 | tmc2130 initialized
//! 0b | 00 | 00 | 00 | 00 | A/D converter initialized
//! b0 | b0 | b0 | b0 | b0 | Error, filament detected, still present
//! 0b | 0b | 0b | 0b | 0b | Error, filament detected, no longer present, continue by right button click
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//! @n b - blinking
void setup()
{
    #warning Add logic to detect the number of connected LEDs here
    numSlots = 3; //NUM_SLOTS_DEFAULT;
    LEDS.setOutput(PIN_LED_DIN);

    //permanentStorageInit();
    startWakeTime = millis();
    
    led_blink(1);

    PORTA |= 0x02; // Set Button ADC Pin High
    servoIdler.attach(PIN_IDL_SERVO);
    
    sei();

    led_blink(2);
    initUart();
    led_blink(3);
    led_blink(4);

    clr_leds();
    homeIdler(true);
    if (active_extruder != numSlots) txPayload((char*)"STR--");

    sendStringToPrinter((char*)"start");



    /*clr_leds();
    for(int i = 0; i < numSlots; i++)
    {
        clr_leds();
        set_led(1 << 2 * (numSlots - 1 - i));
        delay(1000);
        clr_leds();
        set_led(2 << 2 * (numSlots - 1 - i));
        delay(1000);
        clr_leds();
        set_led(3 << 2 * (numSlots - 1 - i));
        delay(1000);
    }*/
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
    clr_leds();
    set_led(1 << 2 * (4 - active_extruder));

    if (!isFilamentLoaded()) 
    {
        switch (buttonClicked()) 
        {
        case ADC_Btn_Right:
            if (active_extruder < numSlots) set_positions(active_extruder + 1, true);
            if (active_extruder == numSlots) txPayload((char*)"X1---");
            break;
        case ADC_Btn_Left:
            if (active_extruder == numSlots) txPayload((char*)"ZZZ--");
            if (active_extruder > 0) set_positions(active_extruder - 1, true);
            break;
        default:
            break;
        }
    } 
    else 
    {
        switch (buttonClicked()) 
        {
          case ADC_Btn_Right:
          case ADC_Btn_Left:
            txPayload((char*)"Z1---");
            delay(1000);
            txPayload((char*)"ZZZ--");
            break;
          default:
            break;
        }
    }

    if (active_extruder == 5) 
    {
        clr_leds();
        set_led(3 << 2 * 0);
        delay(100);
        clr_leds();
        delay(100);
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
        if (ADC_Btn_Middle == buttonClicked()) 
        {
            if (active_extruder < numSlots)
            {
                feed_filament();
            }
            else if (active_extruder == numSlots) 
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
        case ADC_Btn_Right:
            engage_filament_pulley(true);
            moveSmooth(AX_PUL, (PULLEY_EJECT_STEPS * -1), filament_lookup_table[5][filament_type[previous_extruder]], GLOBAL_ACC);
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
/*    if(confirmedPayload)
    {
        char temp[20];
        sprintf(temp,"%c %c %c\n", rxData1, rxData2, rxData3);
        sendData(temp);

        rxData1 = 0;
        rxData2 = 0;
        rxData3 = 0;
        confirmedPayload = false;
    }
    return;*/

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
        if (tData2 < numSlots) 
        {
            m600RunoutChanging = false;
            MMU2SLoading = true;
            toolChange(tData2);
            sendStringToPrinter(OK);
        }
    } 
    else if (tData1 == 'L') 
    {
        // Lx Load Filament CMD Received
        if (tData2 < numSlots) 
        {
            if (isFilamentLoaded()) 
            {
                txPayload((char*)"Z1---");
                delay(1500);
                txPayload((char*)"ZZZ--");
            } 
            else 
            {
                set_positions(tData2, true);
                feed_filament(); // returns OK and active_extruder to update MK3
            }
            sendStringToPrinter(OK);
        }
    } 
    else if ((tData1 == 'U') && (tData2 == '0')) 
    {
        // Ux Unload filament CMD Received
        unload_filament_withSensor();
        homedOnUnload = false; // Clear this flag as unload_filament_withSensor() method uses it within the 'T' cmds
        sendStringToPrinter(OK);
        isPrinting = false;
        toolChanges = 0;
        trackToolChanges = 0;
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
        if ((tData2 < numSlots) && (tData3 < 3)) 
        {
            filament_type[tData2] = tData3;
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
        if (tData2 < numSlots) // Ex: eject filament
        {
            m600RunoutChanging = true;
            eject_filament(tData2);
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
    engage_filament_pulley(false);                    // park the idler stepper motor
    disableStepper(AX_IDL);                            // turn OFF the idler stepper motor

    inErrorState = true;

    while ((ADC_Btn_Middle != buttonClicked()) || isFilamentLoaded()) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        if (!showPrevious) 
        {
            switch (buttonClicked()) 
            {
                case ADC_Btn_Right:
                    engage_filament_pulley(true);
                    if (isFilamentLoaded()) 
                    {
                        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH * 1.5) * -1), filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
                        {
                            moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[active_extruder]], filament_lookup_table[5][filament_type[active_extruder]], GLOBAL_ACC); // move to filament parking position
                        }
                    } 
                    else 
                    {
                        moveSmooth(AX_PUL, -300, filament_lookup_table[5][filament_type[active_extruder]]);
                    }
                    engage_filament_pulley(false);                 
                    disableStepper(AX_IDL);
                    break;
                case ADC_Btn_Left:
                    engage_filament_pulley(true);
                    moveSmooth(AX_PUL, 300, filament_lookup_table[5][filament_type[previous_extruder]]*1.8);
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
                set_led(2 << 2 * (4 - active_extruder));
            } 
            else 
            {
                set_led(1 << 2 * (4 - active_extruder));
            }
        } 
        else 
        {
            switch (buttonClicked()) 
            {
                case ADC_Btn_Right:
                    engage_filament_pulley(true);
                    if (isFilamentLoaded()) 
                    {
                        if (moveSmooth(AX_PUL, ((BOWDEN_LENGTH * 1.5) * -1), filament_lookup_table[5][filament_type[previous_extruder]]*1.8, GLOBAL_ACC, true) == MR_Success)  // move to trigger FINDA
                        {
                            moveSmooth(AX_PUL, filament_lookup_table[3][filament_type[previous_extruder]], filament_lookup_table[5][filament_type[previous_extruder]]*1.8, GLOBAL_ACC); // move to filament parking position
                        }
                    } 
                    else 
                    {
                        moveSmooth(AX_PUL, -300, filament_lookup_table[5][filament_type[previous_extruder]]*1.8);
                    }
                    engage_filament_pulley(false);
                    disableStepper(AX_IDL);
                    break;
                case ADC_Btn_Left:
                    engage_filament_pulley(true);
                    moveSmooth(AX_PUL, 300, filament_lookup_table[5][filament_type[previous_extruder]]*1.8);
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
                set_led(1 << 2 * (4 - active_extruder));
            }
            delay(100);
            if (isFilamentLoaded()) 
            {
                set_led(2 << 2 * (4 - previous_extruder));
            } 
            else 
            {
                set_led(1 << 2 * (4 - previous_extruder));
            }
        }
    }
    delay(100);
    
    inErrorState = false;
    txPayload((char*)"ZZZ--"); // Clear MK3 Message
    home(true); // Home and return to previous active extruder
    trackToolChanges = 0;
}

void fixIdlCrash(void) 
{
    disableStepper(AX_IDL);                            // turn OFF the idler stepper motor
    inErrorState = true;

    while (ADC_Btn_Middle != buttonClicked()) 
    {
        //  wait until key is entered to proceed  (this is to allow for operator intervention)
        delay(100);
        clr_leds();
        delay(100);
        if (isFilamentLoaded()) 
        {
            set_led(2 << 2 * (4 - active_extruder));
        } 
        else set_led(1 << 2 * (4 - active_extruder));
    }
    inErrorState = false;
    set_idler_toLast_positions(active_extruder);
}
