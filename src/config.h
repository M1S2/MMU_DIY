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
#define MAX_SPEED_SEL  6000 // micro steps
#define MAX_SPEED_IDL  5000 // micro steps
#define GLOBAL_ACC    80000 // micro steps / s²


//ADC configuration
#define ADC_Btn_None      0
#define ADC_Btn_Right     4
#define ADC_Btn_Middle    2
#define ADC_Btn_Left      1

#define AX_PUL 0 // Pulley (Filament Drive)
#define AX_SEL 1 // Selector
#define AX_IDL 2 // Idler

#define AX_PUL_STEP_MM_Ratio          19

#define PIN_STP_IDL_HIGH (PORTD |= 0x40)
#define PIN_STP_IDL_LOW (PORTD &= ~0x40)
#define PIN_STP_SEL_HIGH (PORTD |= 0x10)
#define PIN_STP_SEL_LOW (PORTD &= ~0x10)

#define PIN_PUL_STP_HIGH (PORTD |= 0x10)        // Pin PD4
#define PIN_PUL_STP_LOW (PORTD &= ~0x10)        // Pin PD4
#define PIN_PUL_DIR_HIGH (PORTD |= 0x08)        // Pin PD3
#define PIN_PUL_DIR_LOW (PORTD &= ~0x08)        // Pin PD3
#define PIN_PUL_EN_HIGH (PORTD |= 0x04)         // Pin PD2
#define PIN_PUL_EN_LOW (PORTD &= ~0x04)         // Pin PD2

#define TOOLSYNC 100                         // number of tool change (T) commands before a selector resync is performed

// signals (from interrupts to main loop)
#define SIG_ID_BTN 1 // any button changed

// states (<0 =error)
#define STA_INIT 0  // setup - initialization
#define STA_BUSY 1  // working
#define STA_READY 2 // ready - accepting commands

#define STA_ERR_TMC0_SPI -1     // TMC2130 axis0 spi error - not responding
#define STA_ERR_TMC0_MSC -2     // TMC2130 axis0 motor error - short circuit
#define STA_ERR_TMC0_MOC -3     // TMC2130 axis0 motor error - open circuit
#define STA_ERR_TMC0_PIN_STP -4 // TMC2130 axis0 pin wirring error - stp signal
#define STA_ERR_TMC0_PIN_DIR -5 // TMC2130 axis0 pin wirring error - dir signal
#define STA_ERR_TMC0_PIN_ENA -6 // TMC2130 axis0 pin wirring error - ena signal

#define STA_ERR_TMC1_SPI -11     // TMC2130 axis1 spi error - not responding
#define STA_ERR_TMC1_MSC -12     // TMC2130 axis1 motor error - short circuit
#define STA_ERR_TMC1_MOC -13     // TMC2130 axis1 motor error - open circuit
#define STA_ERR_TMC1_PIN_STP -14 // TMC2130 axis1 pin wirring error - stp signal
#define STA_ERR_TMC1_PIN_DIR -15 // TMC2130 axis1 pin wirring error - dir signal
#define STA_ERR_TMC1_PIN_ENA -16 // TMC2130 axis1 pin wirring error - ena signal

#define STA_ERR_TMC2_SPI -21     // TMC2130 axis2 spi error - not responding
#define STA_ERR_TMC2_MSC -22     // TMC2130 axis2 motor error - short circuit
#define STA_ERR_TMC2_MOC -23     // TMC2130 axis2 motor error - open circuit
#define STA_ERR_TMC2_PIN_STP -24 // TMC2130 axis2 pin wirring error - stp signal
#define STA_ERR_TMC2_PIN_DIR -25 // TMC2130 axis2 pin wirring error - dir signal
#define STA_ERR_TMC2_PIN_ENA -26 // TMC2130 axis2 pin wirring error - ena signal

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

// number of extruders
#define EXTRUDERS 5

// CONFIG
//#define _CONFIG_H
