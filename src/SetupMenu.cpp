//! @file

#include "Buttons.h"
#include "SetupMenu.h"
#include "main.h"
#include "led.h"
#include "mmctl.h"
#include "motion.h"
#include "permanent_storage.h"
#include "uart.h"
#include "config.h"

void slotSetupMenuAngle();
void slotSetupMenuBowdenLen();

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Select previous menu option (if not at first option)
//! RIGHT           | Select next menu option (if not at last option)
//! MIDDLE          | Enter menu for slot setup option
//! MIDDLE (long)   | Exit slot setup menu and return to base menu
void slotSetupMenu()
{
    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU, 1000);
    uint8_t _menu = 0;
    bool _exit = false;
    do 
    {
        switch (_menu) 
        {
            case 0: set_led_state(active_extruder, LED_SLOT_SETUP_MENU_ANGLE, 1000); break;
            case 1: set_led_state(active_extruder, LED_SLOT_SETUP_MENU_BOWDEN_LEN, 1000); break;
        }

        if(get_key_press(1 << KEY_LEFT) || get_key_rpt(1 << KEY_LEFT))
        {
            if (_menu > 0) { _menu--; }
        }
        else if(get_key_press(1 << KEY_RIGHT) || get_key_rpt(1 << KEY_RIGHT))
        {
            if (_menu < 1) { _menu++; }
        }
        else if (get_key_long(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
        else if (get_key_press(1 << KEY_MIDDLE))
        {
            switch (_menu) 
            {
            case 0: slotSetupMenuAngle(); break;
            case 1: slotSetupMenuBowdenLen(); break;
            }
        }
    } while (!_exit);

    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU, 1000);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Rotate Idler one step back
//! RIGHT           | Rotate Idler one step forward
//! MIDDLE          | Exit slot setup option angle menu and return to slot setup menu
void slotSetupMenuAngle()
{
    bool _exit = false;
    do 
    {
        set_led_state(active_extruder, LED_SLOT_SETUP_ANGLE, 0);

        if(get_key_press(1 << KEY_LEFT) || get_key_rpt(1 << KEY_LEFT))
        {
            // decrease slot idler angle
            sendStringToPrinter((char*)"slot angle decrease");
            #warning Add functionality here
        }
        else if(get_key_press(1 << KEY_RIGHT) || get_key_rpt(1 << KEY_RIGHT))
        {
            // increase slot idler angle
            sendStringToPrinter((char*)"slot angle increase");
            #warning Add functionality here
        }
        else if (get_key_long(1 << KEY_MIDDLE) || get_key_press(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
    } while (!_exit);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Decrease the bowden length by one step
//! RIGHT           | Increase the bowden length by one step
//! MIDDLE          | Exit slot setup option bowden length menu and return to slot setup menu
void slotSetupMenuBowdenLen()
{
    bool _exit = false;
    do 
    {
        set_led_state(active_extruder, LED_SLOT_SETUP_BOWDEN_LEN, 0);

        sendStringToPrinter((char*)"slot bowden len menu");
        #warning Add functionality here

        if (get_key_long(1 << KEY_MIDDLE) || get_key_press(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
    } while (!_exit);
}




#if false

void settings_bowden_length();

//! @brief Show setup menu
//!
//! Items are selected by left and right buttons, activated by middle button.
//!
//! LED indication of states
//!
//! RG | RG | RG | RG | RG | meaning
//! -- | -- | -- | -- | -- | ------------------------
//! 11 | 00 | 00 | 00 | 01 | initial state, no action
//! 11 | 00 | 00 | 01 | 00 | setup bowden length
//! 11 | 00 | 01 | 00 | 00 | erase EEPROM if unlocked
//! 11 | 01 | 00 | 00 | 00 | unlock EEPROM erase
//! 11 | 00 | 00 | 00 | 00 | exit setup menu
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//!
void setupMenu()
{
    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU, 1200);

    uint8_t _menu = 0;
    bool _exit = false;
    bool eraseLocked = true;
    inErrorState = true;

    do 
    {
        set_led(_menu, COLOR_WHITE);

        switch (buttonClicked()) 
        {
        case BTN_LEFT:
            if (_menu > 0) 
            {
                _menu--;
                _delay_ms(800);
            }
            break;
        case BTN_MIDDLE:
            switch (_menu) 
            {
            case 1:
                if (!isFilamentLoaded()) 
                {
                    settings_bowden_length();
                    _exit = true;
                }
                break;
            case 2:
                if (!eraseLocked)
                {
                    eepromEraseAll();
                    _exit = true;
                }
                break;
            case 3: // unlock erase
                eraseLocked = false;
                break;
            case 4: // exit menu
                set_positions(numSlots - 1, true);
                _exit = true;
                break;
            }
            break;
        case BTN_RIGHT:
            if (_menu < 4) 
            {
                _menu++;
                _delay_ms(800);
            }
            break;
        default:
            break;
        }
    } while (!_exit);

    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU, 400);
    set_led_state(active_extruder, LED_SLOT_SELECTED, 0);
}

//! @brief Set bowden length
//!
//! button         | action
//! -------------- | ------
//! Left   LOADED  | increase bowden length / feed more filament
//! Left UNLOADED  | store bowden length to EEPROM and exit
//! Right          | decrease bowden length / feed less filament
//! Middle         | Load/Unload to recheck length
//!
//! This state is indicated by following LED pattern:
//!
//! RG | RG | RG | RG | RG
//! -- | -- | -- | -- | --
//! bb | 00 | 00 | 0b | 00
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//! @n b - blinking
//!
void settings_bowden_length()
{
    // load filament to end of detached bowden tube to check correct length
    set_positions(0, true);
    enum class S : uint8_t 
    {
        NotExtruded,
        Extruded,
        Done
    };
    S state = S::NotExtruded;
    for (uint8_t i = 0; i < 7; i++)
    {
        bowdenLength.increase();
    }
    do 
    {
        _delay_ms(10);
        switch (buttonClicked()) 
        {
        case BTN_LEFT:
            switch (state) 
            {
            case S::NotExtruded:
                state = S::Done;
                for (uint8_t i = 0; i < 7; i++) 
                {
                    bowdenLength.decrease();
                }
                bowdenLength.~BowdenLength();
                BOWDEN_LENGTH = BowdenLength::get();
                break;
            case S::Extruded:
                if (bowdenLength.increase())
                {
                    move_pulley(bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default:
                break;
            }
            break;
        case BTN_MIDDLE:
            switch (state) 
            {
            case S::NotExtruded:
                state = S::Extruded;
                load_filament_withSensor(bowdenLength.m_length);
                break;
            case S::Extruded:
                state = S::NotExtruded;
                set_led(0, COLOR_WHITE);
                set_led(numSlots - 1, COLOR_BLUE, false);
                _delay_ms(50);
                unload_filament_forSetup(bowdenLength.m_length);
                break;
            default:
                break;
            }
            break;
        case BTN_RIGHT:
            switch (state) 
            {
            case S::NotExtruded:
                break;
            case S::Extruded:
                if (bowdenLength.decrease())
                {
                    move_pulley(-bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    } while (state != S::Done);
}

#endif