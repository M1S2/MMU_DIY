// uart.cpp
#include "uart.h"
#include "config.h"
#include <avr/interrupt.h>
#include "Arduino.h"

volatile unsigned char readRxBuffer, rxData1 = 0, rxData2 = 0, rxData3 = 0;
volatile bool confirmedPayload = false, IR_SENSOR = false;
enum class rx
{
    Data1,
    Data2,
    Data3
};
rx rxCount = rx::Data1;

inline rx& operator++(rx& byte, int)
{
    const int i = static_cast<int>(byte) + 1;
    byte = static_cast<rx>((i) % 7);
    return byte;
}

void initUart()
{
#ifdef ENV_ARDUINO
    //Setup USART0 Interrupt Registers
    UCSR0A = (1 << U2X0); // baudrate multiplier
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (0 << UCSZ02);   // Turn on the transmission and reception circuitry
    UCSR0C = (0 << UMSEL01) | (0 << UMSEL00)| (0 << UPM01) | (0 << UPM00) | (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00); // Use 8-bit character sizes
    UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
    UCSR0B |= (1 << RXCIE0);
#else
    //Setup USART Interrupt Registers
    UCSRA = (1 << U2X); // baudrate multiplier
    UCSRB = (1 << RXEN) | (1 << TXEN) | (0 << UCSZ2);   // Turn on the transmission and reception circuitry
    UCSRC = (0 << UPM1) | (0 << UPM0) | (1 << USBS) | (1 << UCSZ1) | (1 << UCSZ0); // Use 8-bit character sizes
    UBRRH = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRRL = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
    UCSRB |= (1 << RXCIE);
#endif
}

#ifdef ENV_ARDUINO
ISR(USART0_RX_vect)
{
    readRxBuffer = UDR0;
#else
ISR(USART_RXC_vect)
{
    readRxBuffer = UDR;
#endif
    if(readRxBuffer == '\n' || readRxBuffer == '\r')
    {
        if (rxData1 == 'A' && rxCount == rx::Data2)     // received data was "A\n" 
        {
            IR_SENSOR = true;
        }
        else
        {
            confirmedPayload = true;
        }
        rxCount = rx::Data1;
    }
    else
    {
        switch (rxCount) 
        {
            case rx::Data1:
                rxData1 = readRxBuffer;
                rxCount++;
                break;
            case rx::Data2:
                rxData2 = readRxBuffer;
                rxCount++;
                break;
            case rx::Data3:
                rxData3 = readRxBuffer;
                rxCount = rx::Data1;
                break;
        }
    }
}

void sendStringToPrinter(char* str)
{
#ifdef ENV_ARDUINO
    for (uint8_t i = 0; i < strlen(str); i++) 
    {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Do nothing until UDR is ready for more data to be written to it
        UDR0 = (int)str[i];
    }
    loop_until_bit_is_set(UCSR0A, UDRE0); // Do nothing until UDR is ready for more data to be written to it
    UDR0 = (int)'\n';
#else
    for (uint8_t i = 0; i < strlen(str); i++) 
    {
        loop_until_bit_is_set(UCSRA, UDRE); // Do nothing until UDR is ready for more data to be written to it
        UDR = (int)str[i];
    }
    loop_until_bit_is_set(UCSRA, UDRE); // Do nothing until UDR is ready for more data to be written to it
    UDR = (int)'\n';
#endif
}

void txFINDAStatus(void)
{
    char tempFinda[10];
    sprintf(tempFinda, "%hhu%s", isFilamentLoaded(), OK);
    sendStringToPrinter(tempFinda);
}