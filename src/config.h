// config.h - main configuration file

#define FW_VERSION      402 // example: 103 means version 1.0.3
#define FW_BUILDNR      380 // number of commits in 'master'

#define WAKE_TIMER      300000        // 5m

// speeds and accelerations
#define GLOBAL_ACC      80000 // micro steps / s²

//ADC configuration
#define ADC_Btn_None      0
#define ADC_Btn_Right     4
#define ADC_Btn_Middle    2
#define ADC_Btn_Left      1

#define AX_PUL 0 // Pulley (Filament Drive)
#define AX_IDL 1 // Idler

#define AX_PUL_STEP_MM_Ratio          19

#define NUM_SLOTS_MAX       10
#define NUM_SLOTS_DEFAULT   3

#define PIN_LED_DIN     22      //PD7           // Pin PD7
#define PIN_IDL_SERVO   24      //PC4           // Pin PC4

#define PIN_PUL_DIR_HIGH (PORTC |= 0x20)        // Pin PC5
#define PIN_PUL_DIR_LOW (PORTC &= ~0x20)        // Pin PC5
#define PIN_PUL_STP_HIGH (PORTC |= 0x40)        // Pin PC6
#define PIN_PUL_STP_LOW (PORTC &= ~0x40)        // Pin PC6
#define PIN_PUL_EN_HIGH (PORTC |= 0x80)         // Pin PC7
#define PIN_PUL_EN_LOW (PORTC &= ~0x80)         // Pin PC7

#define PIN_FINDA       A0
#define isFilamentLoaded() digitalRead(PIN_FINDA)

#define PIN_BUTTONS     A1

// Type Definitions
// filament types (0: default; 1:flex; 2: PVA)
// Default
#define TYPE_0_MAX_SPPED_PUL                  4000  //  S/S
#define TYPE_0_ACC_FEED_PUL                   5000  //  S/S/S
#define TYPE_0_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_0_FSensor_Sense_STEPS            1500  //  STEPS
#define TYPE_0_FEED_SPEED_PUL                  760  //  S/S
#define TYPE_0_L2ExStageOne                    350  //  S/S
#define TYPE_0_L2ExStageTwo                    440  //  S/S
#define TYPE_0_UnloadSpeed                     750  //  S/S
// Flex
#define TYPE_1_MAX_SPPED_PUL                  2500  //  S/S from 300
#define TYPE_1_ACC_FEED_PUL                   1800  //  S/S/S
#define TYPE_1_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_1_FSensor_Sense_STEPS            3000  //  STEPS
#define TYPE_1_FEED_SPEED_PUL                  650  //  S/S
#define TYPE_1_L2ExStageOne                    350  //  S/S
#define TYPE_1_L2ExStageTwo                    440  //  S/S
#define TYPE_1_UnloadSpeed                     650  //  S/S
// PVA
#define TYPE_2_MAX_SPPED_PUL                  2800  //  S/S
#define TYPE_2_ACC_FEED_PUL                   2000  //  S/S/S
#define TYPE_2_FILAMENT_PARKING_STEPS         -670  //  STEPS
#define TYPE_2_FSensor_Sense_STEPS            1500  //  STEPS
#define TYPE_2_FEED_SPEED_PUL                  760  //  S/S
#define TYPE_2_L2ExStageOne                    350  //  S/S
#define TYPE_2_L2ExStageTwo                    440  //  S/S
#define TYPE_2_UnloadSpeed                     750  //  S/S

// Calibration
#define PULLEY_EJECT_STEPS          2000
#define IDLER_NEXT_FILAMENT_ANGLE   -45     // angle that the idler must rotate between two consecutive filaments
//#define IDLER_HOME_ANGLE_ABSOLUTE   0       // absolute angle in degree at which the idler is homed
#define IDLER_PARK_ANGLE_ABSOLUTE   180     // absolute angle in degree at which the idler is parked. The next filament angle is calculated from this point.

// CONFIG
//#define _CONFIG_H
