/*
 * GccApplication3_AFE.c
 *
 * Created: 5/5/2015 6:12:52 PM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include <util/delay.h>

/*Function prototypes*/
void SPI_MasterInit(void);
void SPI_MasterTransmit(unsigned char cData);
unsigned char SPI_MasterReceive(unsigned char cdata);
void CS2up();
void CS2down();
void AFE_Init_BCM_SS();
void AFE_Init_BCM_Cont();
uint16_t readBCM();
void AFE_Init_WSM_SS();
void AFE_Init_WSM_Cont();
uint16_t readWSM();

/*Definitions*/
#define MOSI DDB5
#define SCK DDB7
#define CLK0 DDB1
#define CS2 DDB2
#define CS1 DDB4
#define RDY DDB0

/*Variables*/
unsigned char msb;
unsigned char lsb;
uint16_t msb_lsb_comb;
uint16_t DATA_BCM;
uint16_t DATA_WSM;


int main(void)
{
    while(1){
		SPI_MasterInit();
		CS2up();	
		
		while(1){
		DATA_BCM=readBCM();
		DATA_WSM=readWSM();
		}		
		
		
    }
}

void SPI_MasterInit(void){
	//Function modified from ATmega 1284P Datasheet
	/* Set MOSI and SCK output, all others input */
	DDRB |= (1<<CS2)|(1<<CS1)|(1<<MOSI)|(1<<SCK);
	
	/* Enable SPI, Master, set clock rate fck/4 -> 250kHz */
	SPCR = (1<<SPE)|(1<<MSTR);
}

void SPI_MasterTransmit(unsigned char cData){
	//Function copied from ATmega 1284P Datasheet
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

unsigned char SPI_MasterReceive(unsigned char cdata){
	SPDR=cdata;
	while(!(SPSR & (1<<SPIF))){
		//do nothing
	}
	/* Return Data Register */
	return SPDR;
}

void CS2up(){
	PORTB |= (1<<DDB2); //Assert PB2 HIGH
}

void CS2down(){
	PORTB &= !(1<<DDB2); //Assert PB2 LOW
}

void AFE_Init_BCM_SS(){
	CS2down();
	
	//Config Code START
	SPI_MasterTransmit(0x02); // Misc Reg 1
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x03); // Misc Reg 2
	SPI_MasterTransmit(0xFF);
	SPI_MasterTransmit(0xFF);
	
	SPI_MasterTransmit(0x09); // Dev Cont 1
	SPI_MasterTransmit(0x60);
	SPI_MasterTransmit(0x06);
	
	SPI_MasterTransmit(0x0A); // ISW Mux
	SPI_MasterTransmit(0x40);
	SPI_MasterTransmit(0x80);
	
	SPI_MasterTransmit(0x0B); // VSense Mux
	SPI_MasterTransmit(0x40);
	SPI_MasterTransmit(0x80);
	
	SPI_MasterTransmit(0x0C); // IQ Mode Enable
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0D); // Weight Scale Control
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0E); // BCM DAC Freq
	SPI_MasterTransmit(0x01);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0F); // Dev Cont 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x10); // ADC Control Reg 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x63);
	
	SPI_MasterTransmit(0x1A); // Misc Reg 3
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x30);
	//Config Code END
	
	//Begin ADC Conversion
	SPI_MasterTransmit(0x01); // ADC Control Reg 1
	SPI_MasterTransmit(0xC1);
	SPI_MasterTransmit(0xD0);
			
	CS2up();
}

void AFE_Init_BCM_Cont(){
	CS2down();
	
	//Config Code START
	SPI_MasterTransmit(0x02); // Misc Reg 1
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x03); // Misc Reg 2
	SPI_MasterTransmit(0xFF);
	SPI_MasterTransmit(0xFF);
	
	SPI_MasterTransmit(0x09); // Dev Cont 1
	SPI_MasterTransmit(0x60);
	SPI_MasterTransmit(0x06);
	
	SPI_MasterTransmit(0x0A); // ISW Mux
	SPI_MasterTransmit(0x40);
	SPI_MasterTransmit(0x80);
	
	SPI_MasterTransmit(0x0B); // VSense Mux
	SPI_MasterTransmit(0x40);
	SPI_MasterTransmit(0x80);
	
	SPI_MasterTransmit(0x0C); // IQ Mode Enable
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0D); // Weight Scale Control
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0E); // BCM DAC Freq
	SPI_MasterTransmit(0x01);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0F); // Dev Cont 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x10); // ADC Control Reg 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x63);
	
	SPI_MasterTransmit(0x1A); // Misc Reg 3
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x30);
	//Config Code END
	
	//Begin ADC Conversion
	SPI_MasterTransmit(0x01); // ADC Control Reg 1
	SPI_MasterTransmit(0x41); // Continuous Conversion w/ Differential Input
	SPI_MasterTransmit(0x50); //<-250 Samples/Sec
	
	CS2up();
}

uint16_t readBCM(){
		AFE_Init_BCM_SS(); //BCM Read Single Shot Mode
			
		//Polling for RDY (PINB0 must transition 1>0)
		while(PINB & 0x01){}
		
		//Read Conversion Data
		CS2down();
		SPI_MasterTransmit(0x20);
		msb=SPI_MasterReceive(0xff);
		lsb=SPI_MasterReceive(0xff);
		CS2up();
		
		//Combine [MSB(8) LSB(8)] into 16bit Val
		msb_lsb_comb=((uint16_t) msb<<8)|lsb;
		return msb_lsb_comb;
	
}

void AFE_Init_WSM_SS(){
	CS2down();
	
	//Config Code START
	SPI_MasterTransmit(0x02); // Misc Reg 1
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x03); // Misc Reg 2
	SPI_MasterTransmit(0xFF);
	SPI_MasterTransmit(0xFF);
	
	SPI_MasterTransmit(0x09); // Dev Cont 1
	SPI_MasterTransmit(0x60);
	SPI_MasterTransmit(0x05);
	
	SPI_MasterTransmit(0x0A); // ISW Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0B); // VSense Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0C); // IQ Mode Enable
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0D); // Weight Scale Control
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00); // 2nd Stage Gain = 1, No Offset DAC Val
	
	SPI_MasterTransmit(0x0E); // BCM DAC Freq
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0F); // Dev Cont 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x10); // ADC Control Reg 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x1A); // Misc Reg 3
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x30);
	//Config Code END
	
	//Begin ADC Conversion
	SPI_MasterTransmit(0x01); // ADC Control Reg 1
	SPI_MasterTransmit(0xC1);
	SPI_MasterTransmit(0xD0);
	
	CS2up();
}

void AFE_Init_WSM_Cont(){
	CS2down();
	
	//Config Code START
	SPI_MasterTransmit(0x02); // Misc Reg 1
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x03); // Misc Reg 2
	SPI_MasterTransmit(0xFF);
	SPI_MasterTransmit(0xFF);
	
	SPI_MasterTransmit(0x09); // Dev Cont 1
	SPI_MasterTransmit(0x60);
	SPI_MasterTransmit(0x05);
	
	SPI_MasterTransmit(0x0A); // ISW Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0B); // VSense Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0C); // IQ Mode Enable
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0D); // Weight Scale Control
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00); // 2nd Stage Gain = 1, No Offset DAC Val
	
	SPI_MasterTransmit(0x0E); // BCM DAC Freq
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0F); // Dev Cont 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x10); // ADC Control Reg 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x1A); // Misc Reg 3
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x30);
	//Config Code END
	
	//Begin ADC Conversion
	SPI_MasterTransmit(0x01); // ADC Control Reg 1
	SPI_MasterTransmit(0x41);
	SPI_MasterTransmit(0x50);
	
	CS2up();
}

uint16_t readWSM(){
		AFE_Init_WSM_SS();
		
		//Polling for RDY (PINB0 must transition 1>0)
		while(PINB & 0x01){}
			
		//Read Conversion Data
		CS2down();
		SPI_MasterTransmit(0x20);
		msb=SPI_MasterReceive(0xff);
		lsb=SPI_MasterReceive(0xff);
		CS2up();
		
		//Combine [MSB(8) LSB(8)] into 16bit Val
		msb_lsb_comb=((uint16_t) msb<<8)|lsb;
		return msb_lsb_comb;
}


