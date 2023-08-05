// config.h - main configuration file

#define FW_VERSION  402 // example: 103 means version 1.0.3
#define FW_BUILDNR  380 // number of commits in 'master'

#define WAKE_TIMER            300000        // 5m

//#define green_board

// shift register outputs
// LEDS - hardcoded
#define SHR16_LEDG0           0x0100
#define SHR16_LEDR0           0x0200
#define SHR16_LEDG1           0x0400
#define SHR16_LEDR1           0x0800
#define SHR16_LEDG2           0x1000
#define SHR16_LEDR2           0x2000
#define SHR16_LEDG3           0x4000
#define SHR16_LEDR3           0x8000
#define SHR16_LEDG4           0x0040
#define SHR16_LEDR4           0x0080
#define SHR16_LED_MSK         0xffc0

// UART0 (USB)
#define UART0_BDR 115200

// speeds and accelerations
#define GLOBAL_ACC    80000 // micro steps / s²


//ADC configuration
#define ADC_Btn_None      0
#define ADC_Btn_Right     4
#define ADC_Btn_Middle    2
#define ADC_Btn_Left      1

#define AX_PUL 0 // Pulley (Filament Drive)
#define AX_IDL 1 // Idler

#define AX_PUL_STEP_MM_Ratio          19

#define NUM_SLOTS_MAX       10
#define NUM_SLOTS_DEFAULT   5

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

// signals (from interrupts to main loop)
#define SIG_ID_BTN 1 // any button changed

// states (<0 =error)
#define STA_INIT 0  // setup - initialization
#define STA_BUSY 1  // working
#define STA_READY 2 // ready - accepting commands

// Type Definitions
// filament types (0: default; 1:flex; 2: PVA)
// NOTE: Stealth Mode cuts MAX PUL SPEED.
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
#define IDLER_NEXT_FILAMENT_STEPS   30      // angle that the idler must rotate between two consecutive filaments
#define IDLER_HOME_ANGLE_ABSOLUTE   0       // absolute angle in degree at which the idler is homed

// CONFIG
//#define _CONFIG_H
