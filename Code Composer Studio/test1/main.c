#include <msp430.h>
//USING MSP430G2452

/*
 * main.c
 */

//Prototypes
void set_P1_bit(int position, int value);
unsigned int get_P1_bit(int position);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P1DIR = 0x41; //1.0, 1.6 Output, 1.3 and rest Input
    //P1DIR = 8 bit Port Direction - b1: OUT, b0: IN
    //P1OUT = 8 bit Port Logic - b1: HIGH, b0: LOW
    volatile unsigned int a,b;

	while(1){
		set_P1_bit(0,1);
		set_P1_bit(6,0);
		_delay_cycles(100000);
		set_P1_bit(0,0);
		set_P1_bit(6,1);
		_delay_cycles(100000);

	}
}

void set_P1_bit(int position, int value)
{
	// Sets or clears the bit in position 'position'
	// either high or low (1 or 0) to match 'value'.
	// Leaves all other bits in P1 unchanged.

	if (value == 0){ P1OUT &= ~(1 << position); } // Set bit # 'position' low
	else { P1OUT |= (1 << position); } // Set bit # 'position' high

}

unsigned int get_P1_bit(int position)
{
	// gets the bit in position 'position'
	// either high or low (1 or 0) to match 'value'.
	// Leaves all bits in P1 unchanged.

	return (P1OUT & (1<<position));

}
