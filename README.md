# MMU_DIY

Firmware for a self designed Multi Material Unit inspired by the Prusa MMU2S

## LED Indicators
Each LED indication should have a unique length of e.g. 300ms. This is sometimes used as delay between button queries. 

E.g. solid GREEN:\
set_led GREEN\
delay(300)

E.g. blink GREEN:\
set_led GREEN\
delay(150)\
clr_led()\
delay(150)

### Slot LEDs
- Slot selected (manual): GREEN
- Slot operation active (e.g. tool change, ...): BLUE
- Slot error, filament present: RED BLINKING
- Slot error, filament missing: GREEN BLINKING

### Slot Setup LEDs
- Enter/Exit Slot Setup menu: ALL LEDS WHITE BLINK 1x
- Slot Setup menu selection configure slot angle: WHITE 1 PULSE, PAUSE
- Slot Setup menu selection configure bowden length: WHITE 2 PULSES, PAUSE
- Slot Setup configure slot angle: WHITE
- Slot Setup configure bowden length: WHITE

### Global Setup Menu
- Delete EEPROM menu selection: ALL LEDS RED BLINKING FAST
- EEPROM was deleted: ALL LEDS RED
