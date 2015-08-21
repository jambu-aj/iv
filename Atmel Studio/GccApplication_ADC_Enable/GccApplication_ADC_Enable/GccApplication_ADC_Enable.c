/*
 * GccApplication2.c
 *
 * Created: 1/16/2015 4:23:34 PM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
uint8_t adcVal=0;
	
	
int main(void)
{
	// Enabling PB3 LED
	DDRB = 0b00001000; 
	
	//Enabling ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enabling ADC frequency @ 125kHz sample rate for 16Mhz clock
	ADMUX |= (1 << REFS0); // Set ADC reference (AREF) to AVCC
	ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading
	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADSC); // Start A2D Conversions
	
	// initializing interrupt functionality
	/*
	ADCSRA |= (1 << ADIE);
	ADCSRA = 0b10011111;
	sei();
	ISR(ADC_vect){
	*/
	
	for (;;){
		adcVal=ADCH;
		if(adcVal < 128){
			set_PORTB_bit(3, 1);    // Set PB3 high
			ADCSRA |= (1 << ADSC); // Required before reading a single ADCH value
		}
		else{
			set_PORTB_bit(3, 0);    // Set PB3 low
			ADCSRA |= (1 << ADSC); // Start A2D Conversions
		}
	}
}

int wait_one_second(void){
	_delay_ms(1000);
	return 1;
}

int set_PORTB_bit(int position, int value)
{
	// Sets or clears the bit in position 'position'
	// either high or low (1 or 0) to match 'value'.
	// Leaves all other bits in PORTB unchanged.
	
	if (value == 0)
	{
		PORTB &= ~(1 << position);      // Set bit # 'position' low
	}
	else
	{
		PORTB |= (1 << position);       // Set bit # 'position' high
	}
	return 1;
}
