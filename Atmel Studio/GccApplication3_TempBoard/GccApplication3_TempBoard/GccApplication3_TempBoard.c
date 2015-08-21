/*
 * GccApplication2.c
 *
 * Created: 1/23/2015 3:09:34 PM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
uint8_t adcl_val=0;
uint8_t adch_val=0;
	
	
int main(void)
{
	// Enabling PD6 LED
	DDRD = 0b01000000; 
	
	//Enabling ADC
	//ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading
	//ADMUX |= 0x00; // Enable RTD2 (Pin ADC0)
	//ADMUX |= 0x01; // Enable RTD1 (Pin ADC1)
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC // Start A2D Conversions // ADC frequency @ 125kHz sample rate for 1Mhz clock (x8 Division Factor) 
	
	// initializing interrupt functionality
	/*
	ADCSRA |= (1 << ADIE);
	ADCSRA = 0b10011111;
	sei();
	ISR(ADC_vect){
	*/
	
	
	
	for (;;){
		adcl_val=ADCL;
		adch_val=ADCH;
		if(adcl_val > 237){
			set_PORTD_bit(6, 1);    // Set PB3 high
			ADCSRA |= (1 << ADSC); // Required before reading a single ADCH value
		}
		else{
			set_PORTD_bit(6, 0);    // Set PB3 low
			ADCSRA |= (1 << ADSC); // Start A2D Conversions
		}
	}
	
}

int wait_one_second(void){
	_delay_ms(1000);
	return 1;
}

int set_PORTD_bit(int position, int value)
{
	// Sets or clears the bit in position 'position'
	// either high or low (1 or 0) to match 'value'.
	// Leaves all other bits in PORTB unchanged.
	
	if (value == 0)
	{
		PORTD &= ~(1 << position);      // Set bit # 'position' low
	}
	else
	{
		PORTD |= (1 << position);       // Set bit # 'position' high
	}
	return 1;
}
