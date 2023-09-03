// slotDetection.cpp

#include "slotDetection.h"
#include "config.h"
#include <avr/io.h>
#include "main.h"

#include "uart.h"

/**
 * @brief Detect the number of available slot
 * This is done, using a common resistor on the control PCB (R0) and resistors on the slot PCB (Rn) forming a resistor divider.
 *
 *  +5V
 *   |
 *  | | R0
 *   |
 *   |----------------------------------------> U_meas
 *      |           |           |
 *     | | Rn      | | Rn      | | Rn   ...
 *      |           |           |
 *     ---         ---         ---
 *     GND         GND         GND
 * 
 * @return: number of detected slots or NUM_SLOTS_DEFAULT if no slots are detected
 */
int detect_numSlots(void)
{
    uint16_t adc_val = 0;
    for (int i = 0; i < 4; i++) adc_val += analogRead(PIN_R_DET);
    adc_val = adc_val / 4;

    // n = (Rn / R0) * ((VCC / Umeas) - 1)      with Rn = Resistor on slot PCB, R0 = common resistor on control board, VCC corresponds to max. ADC value 1023
    int n = round((SLOT_DETECTION_SLOT_RESISTOR / (float)SLOT_DETECTION_COMMON_RESISTOR) * ((1023 / (float)adc_val) - 1)); 

    return n == 0 ? NUM_SLOTS_DEFAULT : n;
}