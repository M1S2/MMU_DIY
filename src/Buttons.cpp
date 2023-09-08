//! @file

#include "Buttons.h"
#include "config.h"
//#include "uart.h"

//! @brief Is button pushed?
//! we use an analog input with different DC-levels for each button
//!
//! @return button pushed
uint8_t buttonClicked()
{
    uint8_t trys = 2;
    uint8_t button = BTN_NONE;
    do
    {    
        uint16_t btn_adc_val = 0;
        for (int i=0; i < 4; i++) btn_adc_val += analogRead(PIN_BUTTONS);
        btn_adc_val = btn_adc_val / 4;

        if (btn_adc_val > (BTN_LEFT_ADC_VALUE - BTN_VALID_ADC_DIFF) && btn_adc_val < (BTN_LEFT_ADC_VALUE + BTN_VALID_ADC_DIFF))
        {
            button = BTN_LEFT;
        }
        else if (btn_adc_val > (BTN_MIDDLE_ADC_VALUE - BTN_VALID_ADC_DIFF) && btn_adc_val < (BTN_MIDDLE_ADC_VALUE + BTN_VALID_ADC_DIFF))
        {
            button = BTN_MIDDLE;
        }
        else if (btn_adc_val < (BTN_RIGHT_ADC_VALUE + BTN_VALID_ADC_DIFF))
        {
            button = BTN_RIGHT;
        }

#if false
        // Test Code to show the btn_adc_val on the Monitor to set the correct values in the config.h
        char tmp[50];
        sprintf(tmp, "btn_adc_val=%d", btn_adc_val);
        sendStringToPrinter(tmp);
#endif

        trys--;
        _delay_ms(10);      // debouce then re-read
    } while ((trys > 0) && !button);
    return button;
}
