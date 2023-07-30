// uart.cpp
#include "uart.h"
#include "config.h"
#include "main.h"

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
    //Setup USART0 Interrupt Registers
    UCSR0A = (1 << U2X0); // baudrate multiplier
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (0 << UCSZ02);   // Turn on the transmission and reception circuitry
    UCSR0C = (0 << UMSEL01) | (0 << UMSEL00)| (0 << UPM01) | (0 << UPM00) | (1 << USBS0) | (1 << UCSZ01) | (1 << UCSZ00); // Use 8-bit character sizes
    UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
    UCSR0B |= (1 << RXCIE0);
}

ISR(USART0_RX_vect)
{
    readRxBuffer = UDR0;

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

void sendData(const char* str)
{
    for (uint8_t i = 0; i < strlen(str); i++) 
    {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Do nothing until UDR is ready for more data to be written to it
        UDR0 = (int)str[i];
    }
}

void txPayload(unsigned char payload[])
{
    //loop_until_bit_is_set(UCSR1A, UDRE0);     // Do nothing until UDR is ready for more data to be written to it
    //UDR0 = 0x7F;
    for (uint8_t i = 0; i < 5; i++) 
    {
        loop_until_bit_is_set(UCSR0A, UDRE0); // Do nothing until UDR is ready for more data to be written to it
        UDR0 = (0xFF & (int)payload[i]);
    }
    //loop_until_bit_is_set(UCSR0A, UDRE0);     // Do nothing until UDR is ready for more data to be written to it
    //UDR0 = 0xF7;
}

void txFINDAStatus(void)
{
    //loop_until_bit_is_set(UCSR0A, UDRE0);     // Do nothing until UDR is ready for more data to be written to it
    //UDR0 = 0x06;
    loop_until_bit_is_set(UCSR0A, UDRE0); // Do nothing until UDR is ready for more data to be written to it
    UDR0 = (uint8_t)isFilamentLoaded();
    //loop_until_bit_is_set(UCSR0A, UDRE0);     // Do nothing until UDR is ready for more data to be written to it
    //UDR0 = 0xF7;
}