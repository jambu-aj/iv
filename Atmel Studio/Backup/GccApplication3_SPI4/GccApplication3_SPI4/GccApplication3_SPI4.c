/*
 * GccApplication3_SPI4.c
 *
 * Created: 2/25/2015 12:33:46 PM
 *  Author: andyjambu
 */ 


//#include "SDSettings.h"
#include <avr/io.h>
#include <util/delay.h>

unsigned char msb;
unsigned char lsb;
uint16_t real;
uint16_t AFEReceive();//Function Prototype only
unsigned char SPIReceive();//Function Prototype only
unsigned char SPITranceive(unsigned char data);// proto
#define RDY PORTB0
char val;
char huh;
char huh2;
uint8_t status;
int main(void)
{
	//while(1)
	//{
		DDRD = 0b01100000;  // Enabling PB3
		Init_SPISystem(); //Set up SPI Protocol
		CSup(); //Need to pull it UP after setting it as an Output
		CSdown();
			
		    	
			//	
			AFETransmit(0x020000);//Misc Reg 1
			
			//
			AFETransmit(0x03FFFF);//Misc Reg 2
			AFETransmit(0x096006);//Device Control 1
			AFETransmit(0x0A8040);//ISW_Mux
			AFETransmit(0x0B8040);//VSENSE_Mux
			//AFETransmit(0x0C0000);//I/Q Mode Enable
			//AFETransmit(0x0D0000);//Weight Scale Control
			AFETransmit(0x0E00FF);//BCM_DAC_FREQ
			//AFETransmit(0x0F0000);//Device Control 2
			AFETransmit(0x100063);//ADC Control Register 2
			AFETransmit(0x1A0030);//Misc Reg 3
			
		CSup();
		//while(1){
			CSdown();	
			SPITransmit(0x01);//ADC Control Register 1
			SPITransmit(0xC9);
			SPITransmit(0xC0); //f0?
			CSup();
			
		
			while(status==0){
				set_PORTD_bit(6,1);
				status=PINB0;
			}				
						
			
			CSdown();
			SPITransmit(0x20);
			huh=SPIReceive();
			huh2=SPIReceive();
			CSup();
			
			exit(-1);
	
			/*
			SPITransmit(0x20);//Read address
			msb=SPIReceive();
			SPITransmit(0x00);//MSB plz
			lsb=SPIReceive();
			SPITransmit(0x00);//LSB plz
			lsb=SPIReceive();
			*/
			
			
		//}
		
	//}
}

/*
************************************************************************************************************
AFE/SPI FUNCTIONS-------------------------------------------------------------------------------------------
************************************************************************************************************
*/


void Init_SPISystem(void)
{
	

	/* Set MOSI and SCK and CS2 and CS1 and CLK0 and RDY output, all others input */
	
	DDRB |= (1<<PORTB5)|(1<<PORTB7)|(1<<PORTB2)|(1<<PORTB4)|(1<<PORTB1)|(1<<PORTB0);
	/* Enable SPI, Master, sclk is Fosc/4 -> 250Khz (Fosc = 1Mhz)*/
	SPCR = (1<<SPE) | (1<<MSTR); //Pulls up the MOSI to indicate that ATmega is MASTER
	
	

}

void Init_AFE_BCM(void)
{
				AFETransmit(0x014140);//ADC Control Register 1
				AFETransmit(0x020000);//Misc Reg 1
				AFETransmit(0x03FFFF);//Misc Reg 2
				AFETransmit(0x096006);//Device Control 1
				AFETransmit(0x0A8040);//ISW_Mux
				AFETransmit(0x0B8040);//VSENSE_Mux
				AFETransmit(0x0E0001);//BCM_DAC_FREQ
				AFETransmit(0x100063);//ADC Control Register 2
				AFETransmit(0x1A0030);//Misc Reg 3
				
				/* Continuous mode
				 AFETransmit(0x015150);//ADC Control Register 1
				 
				 AFETransmit(0x020000);//Misc Reg 1
				 AFETransmit(0x03FFFF);//Misc Reg 2
				 AFETransmit(0x1A0030);//Misc Reg 3
				 AFETransmit(0x096006);//Device Control 1
				 
				 AFETransmit(0x0A8040);//ISW_Mux
				 AFETransmit(0x0B8040);//VSENSE_Mux
				 AFETransmit(0x0C0000);//I/Q Mode Enable
				 AFETransmit(0x0D0000);//Weight Scale Control
				 AFETransmit(0x0E00FF);//BCM_DAC_FREQ
				 AFETransmit(0x0F0000);//Device Control 2
				 AFETransmit(0x100063);//ADC Control Register 2
				 */
	
}

void AFETransmit(unsigned long data){ 
	//Sends 3 Byte (24Bit) value as 3 separate 8 bit Char's to SPITransmit() Fn.
	//CSdown();
	SPITransmit(data);//1st byte
	SPITransmit(data<<8);//2nd byte
	SPITransmit(data<<16);//3rd byte
	//msb=SPIReceive();
	//CSup();
}


void SPITransmit(unsigned char data){
	SPDR=data;
	while(!(SPSR & (1<<SPIF))){
		//do nothing
	}
}

unsigned char SPIReceive(){
	/* Wait for reception complete */
	SPDR=0xff;
	while(!(SPSR & (1<<SPIF)));
	/* Return Data Register */
	return SPDR;
}

unsigned char SPITranceive(unsigned char data){
	/* Wait for reception complete */
	SPDR=data;
	while(!(SPSR & (1<<SPIF)));
	/* Return Data Register */
	return SPDR;
}

void CSup(){
	// chip select line UP
	PORTB |= (1<<PORTB2);
}

void CSdown(){
	// chip select line DOWN
	PORTB &= ~(1<<PORTB2);
}

/*
************************************************************************************************************
LED FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

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
