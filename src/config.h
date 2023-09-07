// config.h - main configuration file

#define FW_VERSION      402 // example: 103 means version 1.0.3
#define FW_BUILDNR      380 // number of commits in 'master'

#define WAKE_TIMER      300000        // 5m

//Button configuration
#define BTN_RIGHT               4
#define BTN_MIDDLE              2
#define BTN_LEFT                1
#define BTN_NONE                0
#define BTN_RIGHT_ADC_VALUE     0       // Button connects Pin to GND directly
#define BTN_MIDDLE_ADC_VALUE    93      // Resistor divider with 15k to +5V and 1,5k to GND --> value = (1,5k / (15k + 1,5k)) * 1023
#define BTN_LEFT_ADC_VALUE      171     // Resistor divider with 15k to +5V and 2*1,5k to GND --> value = (3k / (15k + 3k)) * 1023
#define BTN_VALID_ADC_DIFF      20      // Difference around the button ADC values that is regarded as button press (valid range = ADC_VALUE +- ADC_DIFF)

#define AX_PUL 0 // Pulley (Filament Drive)
#define AX_IDL 1 // Idler

#define AX_PUL_STEP_MM_Ratio            19

#define NUM_SLOTS_MAX                   10
#define NUM_SLOTS_DEFAULT               3
#define SLOT_DETECTION_COMMON_RESISTOR  10000
#define SLOT_DETECTION_SLOT_RESISTOR    100000

#ifdef ENV_ARDUINO
#define PIN_LED_DIN     22
#define PIN_IDL_SERVO   24
#define PIN_LED_DEBUG   26
#else
#define PIN_LED_DIN     PIN_PD7
#define PIN_IDL_SERVO   PIN_PC4
#define PIN_LED_DEBUG   PIN_PD2 
#endif

#define PIN_PUL_DIR_HIGH (PORTC |= 0x20)        // Pin PC5
#define PIN_PUL_DIR_LOW (PORTC &= ~0x20)        // Pin PC5
#define PIN_PUL_STP_HIGH (PORTC |= 0x40)        // Pin PC6
#define PIN_PUL_STP_LOW (PORTC &= ~0x40)        // Pin PC6
#define PIN_PUL_EN_HIGH (PORTC |= 0x80)         // Pin PC7
#define PIN_PUL_EN_LOW (PORTC &= ~0x80)         // Pin PC7

#define PIN_FINDA       A0
#define isFilamentLoaded() digitalRead(PIN_FINDA)

#define PIN_BUTTONS     A1
#define PIN_R_DET       A2                      // Pin for detecting the number of available slots

// speeds and accelerations
#define GLOBAL_ACC                          2000 // micro steps / s²
#define MIN_SPEED_PUL                       100

// Type Definitions
// filament types (0: default; 1:flex; 2: PVA)
// Default
#define TYPE_0_MAX_SPEED_PUL                  4000  //  S/S
#define TYPE_0_ACC_FEED_PUL                   5000  //  S/S/S
#define TYPE_0_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_0_FSENSOR_SENSE_STEPS            1500  //  STEPS
#define TYPE_0_FEED_SPEED_PUL                  760  //  S/S
#define TYPE_0_L2EXSTAGEONE                    350  //  S/S
#define TYPE_0_L2EXSTAGETWO                    440  //  S/S
#define TYPE_0_UNLOAD_SPEED                    750  //  S/S
// Flex
#define TYPE_1_MAX_SPEED_PUL                  2500  //  S/S from 300
#define TYPE_1_ACC_FEED_PUL                   1800  //  S/S/S
#define TYPE_1_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_1_FSENSOR_SENSE_STEPS            3000  //  STEPS
#define TYPE_1_FEED_SPEED_PUL                  650  //  S/S
#define TYPE_1_L2EXSTAGEONE                    350  //  S/S
#define TYPE_1_L2EXSTAGETWO                    440  //  S/S
#define TYPE_1_UNLOAD_SPEED                     650  //  S/S
// PVA
#define TYPE_2_MAX_SPEED_PUL                  2800  //  S/S
#define TYPE_2_ACC_FEED_PUL                   2000  //  S/S/S
#define TYPE_2_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_2_FSENSOR_SENSE_STEPS            1500  //  STEPS
#define TYPE_2_FEED_SPEED_PUL                  760  //  S/S
#define TYPE_2_L2EXSTAGEONE                    350  //  S/S
#define TYPE_2_L2EXSTAGETWO                    440  //  S/S
#define TYPE_2_UNLOAD_SPEED                     750  //  S/S

// Calibration
#define PULLEY_EJECT_STEPS          2000
#define IDLER_NEXT_FILAMENT_ANGLE   -45     // angle that the idler must rotate between two consecutive filaments
//#define IDLER_HOME_ANGLE_ABSOLUTE   0       // absolute angle in degree at which the idler is homed
#define IDLER_PARK_ANGLE_ABSOLUTE   180     // absolute angle in degree at which the idler is parked. The next filament angle is calculated from this point.

// CONFIG
//#define _CONFIG_H
