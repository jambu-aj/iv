/*
 * GccApplication1_BT.c
 *
 * Created: 10/19/2015 2:28:56 PM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
unsigned char USART_Receive( void );
int main(void)
{
	//unsigned char ret;
	USART_Init(6); //BR 9600
	//Must set BlueSMIRF to 9600 by entering AT command mode (Power BlueSmirf and use CoolTerm to enter)
	//Note:http://forum.arduino.cc/index.php?topic=62759.0
	//Note:https://www.sparkfun.com/datasheets/Wireless/Bluetooth/rn-bluetooth-um.pdf
	
    while(1)
    {
        //TODO:: Please write your application code 
		USART_Transmit(0xA1);
    }
}

void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
	/* Set synchronous Mode */
	UCSR0A = (0<<U2X0);
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
		;
	/* Get and return received data from buffer */
	return UDR0;
}