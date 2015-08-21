/*
 * GccApplication3_LED.c
 *
 * Created: 5/3/2015 3:52:30 PM
 *  Author: andyjambu
 */ 

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{


	DDRD = 0b01100000;  // Enabling PB3

	for ( ; 1==1 ; )
	{

	  set_PORTD_bit(6, 1);    // high
	  _delay_ms(1000);
	  set_PORTD_bit(6, 0);    // low
	  _delay_ms(1000);
  }

	//return 1;
}

int wait_one_second(void)
{
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
