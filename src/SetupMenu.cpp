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
    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU);
    uint8_t _menu = 0;
    bool _exit = false;
    do 
    {
        switch (_menu) 
        {
            case 0: set_led_state(active_extruder, LED_SLOT_SETUP_MENU_ANGLE); break;
            case 1: set_led_state(active_extruder, LED_SLOT_SETUP_MENU_BOWDEN_LEN); break;
        }

        if(get_key_short(1 << KEY_LEFT) || get_key_rpt(1 << KEY_LEFT))
        {
            if (_menu > 0) { _menu--; }
        }
        else if(get_key_short(1 << KEY_RIGHT) || get_key_rpt(1 << KEY_RIGHT))
        {
            if (_menu < 1) { _menu++; }
        }
        else if (get_key_long(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
        else if (get_key_short(1 << KEY_MIDDLE))
        {
            switch (_menu) 
            {
            case 0: slotSetupMenuAngle(); break;
            case 1: slotSetupMenuBowdenLen(); break;
            }
        }
    } while (!_exit);

    set_led_state(0, LED_ENTER_SLOT_SETUP_MENU);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Rotate Idler one step back (or 5 degree steps on long hold)
//! RIGHT           | Rotate Idler one step forward (or 5 degree steps on long hold)
//! MIDDLE          | Exit slot setup option angle menu and return to slot setup menu
void slotSetupMenuAngle()
{
    bool _exit = false;
    
    set_led_state(active_extruder, LED_SLOT_SETUP_ANGLE);

    do 
    {
        if(get_key_rpt(1 << KEY_LEFT))
        {
            // decrease slot idler angle by 5 degree
            if(idlerSlotAngles[active_extruder] >= 5)
            { 
                idlerSlotAngles[active_extruder] -= 5;
                setIDL2pos(active_extruder);
            }
            else
            { 
                idlerSlotAngles[active_extruder] = 0;
                setIDL2pos(active_extruder);
            }
        }
        else if(get_key_short(1 << KEY_LEFT))
        {
            // decrease slot idler angle by 1 degree
            if(idlerSlotAngles[active_extruder] > 0)
            { 
                idlerSlotAngles[active_extruder]--;
                setIDL2pos(active_extruder);
            }
        }
        else if(get_key_rpt(1 << KEY_RIGHT))
        {
            // increase slot idler angle by 5 degree
            if(idlerSlotAngles[active_extruder] <= 175)
            { 
                idlerSlotAngles[active_extruder] += 5;
                setIDL2pos(active_extruder);
            }
            else
            { 
                idlerSlotAngles[active_extruder] = 180;
                setIDL2pos(active_extruder);
            }
        }
        else if(get_key_short(1 << KEY_RIGHT))
        {
            // increase slot idler angle by 1 degree
            if(idlerSlotAngles[active_extruder] < 180)
            { 
                idlerSlotAngles[active_extruder]++;
                setIDL2pos(active_extruder);
            }
        }
        else if (get_key_long(1 << KEY_MIDDLE) || get_key_press(1 << KEY_MIDDLE))
        {
            _exit = true;
        }
    } while (!_exit);
}

//! button          | action
//! --------------- | ----------------------------------
//! LEFT            | Decrease the bowden length by one step (only if the filament is down the bowden tube)
//! RIGHT           | Increase the bowden length by one step (only if the filament is down the bowden tube)
//! MIDDLE          | Move the filament down or up the bowden tube to check the configured length
//! MIDDLE (long)   | Exit slot setup option bowden length menu and return to slot setup menu
void slotSetupMenuBowdenLen()
{
    set_led_state(active_extruder, LED_SLOT_SETUP_BOWDEN_LEN);

    // load filament to end of detached bowden tube to check correct length
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
        if (get_key_short(1 << KEY_LEFT))
        {
            switch (state)
            {
            case S::NotExtruded: break;
            case S::Extruded:
            bowdenLength.stepSize = BOWDEN_LENGTH_NORMAL_STEPS;
                if (bowdenLength.increase())
                {
                    move_pulley(bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default: break;
            }
        }
        else if (get_key_rpt(1 << KEY_LEFT))
        {
            switch (state)
            {
            case S::NotExtruded: break;
            case S::Extruded:
                bowdenLength.stepSize = BOWDEN_LENGTH_BIG_STEPS;
                if (bowdenLength.increase())
                {
                    move_pulley(bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default: break;
            }
        }
        else if (get_key_short(1 << KEY_RIGHT))
        {
            switch (state) 
            {
            case S::NotExtruded: break;
            case S::Extruded:
            bowdenLength.stepSize = BOWDEN_LENGTH_NORMAL_STEPS;
                if (bowdenLength.decrease())
                {
                    move_pulley(-bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default: break;
            }
        }
        else if (get_key_long(1 << KEY_RIGHT))
        {
            switch (state) 
            {
            case S::NotExtruded: break;
            case S::Extruded:
            bowdenLength.stepSize = BOWDEN_LENGTH_BIG_STEPS;
                if (bowdenLength.decrease())
                {
                    move_pulley(-bowdenLength.stepSize);
                    _delay_ms(200);
                }
                break;
            default: break;
            }
        }
        else if (get_key_short(1 << KEY_MIDDLE))
        {
            switch (state) 
            {
            case S::NotExtruded:
                state = S::Extruded;
                load_filament_withSensor(bowdenLength.m_length);
                set_led_state(active_extruder, LED_SLOT_SETUP_BOWDEN_LEN);
                break;
            case S::Extruded:
                state = S::NotExtruded;
                unload_filament_forSetup(bowdenLength.m_length);
                set_led_state(active_extruder, LED_SLOT_SETUP_BOWDEN_LEN);
                break;
            default: break;
            }
        }
        else if (get_key_long(1 << KEY_MIDDLE))
        {
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
            case S::Extruded: break;
            default: break;
            }
        }
    } while (state != S::Done);
}