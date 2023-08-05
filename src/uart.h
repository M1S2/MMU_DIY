// uart.h
#ifndef _UART_H
#define _UART_H

#define USART_BAUDRATE  115200UL
#define MMU2S_F_CPU     16000000UL
#define BAUD_PRESCALE   (((MMU2S_F_CPU / (USART_BAUDRATE * 8UL))) - 1)
#define OK              (char*)"ok"
#define BLK             0x2D  // Blank data filler

#include <inttypes.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h"

extern volatile unsigned char rxData1, rxData2, rxData3, rxData4, rxData5;
extern volatile bool confirmedPayload, IR_SENSOR;

void initUart();
void sendStringToPrinter(char* str);                 // send the string (with additional newline characters) to the printer
extern void txPayload(char*);
extern void txFINDAStatus(void);

#endif //_UART_H