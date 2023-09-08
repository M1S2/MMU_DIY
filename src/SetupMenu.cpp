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
    clr_leds();
    _delay_ms(200);
    clr_leds();
    for (int i = 0; i < numSlots; i++)
    {
        set_led(i, COLOR_WHITE, false);
    }
    _delay_ms(1200);
    clr_leds();

    uint8_t _menu = 0;
    uint8_t _menu_last_cycle = 10;
    bool _exit = false;
    bool eraseLocked = true;
    inErrorState = true;

    do 
    {
        set_led(_menu, COLOR_WHITE);
        if (_menu != _menu_last_cycle) 
        {
            if (_menu == 0) txPayload((char*)"X1---");
            else if (_menu == 1) txPayload((char*)"X2---");
            else if (_menu == 2) txPayload((char*)"X5---");
            else if (_menu == 3) txPayload((char*)"X6---");
            else if (_menu == 4) txPayload((char*)"X7---");
        }
        _menu_last_cycle = _menu;

        switch (buttonClicked()) 
        {
        case BTN_RIGHT:
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
                txPayload((char*)"ZZR--");
                _exit = true;
                break;
            }
            break;
        case BTN_LEFT:
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

    clr_leds();
    _delay_ms(400);
    clr_leds();
    for (int i = 0; i < numSlots; i++)
    {
        set_led(i, COLOR_WHITE, false);
    }
    _delay_ms(400);
    clr_leds();
    set_led(active_extruder, COLOR_GREEN);
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