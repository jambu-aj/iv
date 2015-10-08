/*
 * GccApplication1_IVFW2.c
 *
 * Created: 9/14/2015 10:49:36 PM
 *  Author: andyjambu
 */ 

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <stdbool.h>
#include "uart.h"
#include "ff.h"
#include "diskio.h"

/*Definitions*/
#define MOSI DDB5
#define SCK DDB7
#define CLK0 DDB1
#define CS2 DDB2 //AFE
#define CS1 DDB4 //SD
#define RDY DDB0
#define LED1 DDD5
#define LED2 DDD6

FIL file;                                               /* Opened file object */
FATFS fatfs;                                            /* File system object */
DIR dir;                                                /* Directory object   */
FRESULT errCode;                                        /* Error code object  */
FRESULT res;                                            /* Result object      */
UINT bytesRead;                                         /* Bytes read object  */
UINT read;                                              /* Read bytes object  */

/*Variables*/

uint16_t DATA_RTD1; //RTD Variables
uint16_t DATA_RTD2;
uint16_t adcvalComb, adcvalComb2;//ADC Combined Values
uint8_t adcl_val, adch_val, adcl_val2, adch_val2;//ADC MSB LSB Values

uint8_t msb; //AFE Variables
uint8_t lsb;
uint16_t msb_lsb_comb;
uint16_t DATA_BCM;
uint16_t DATA_WSM;

unsigned char r1, r2, r7; // SD R/W Flags
char tester;
uint16_t toSend[4];
uint16_t toReceive[4];
uint8_t j=0; //SD Sector #
char arrFill[512];
int result=1;


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
uint16_t rtd1Val();
uint16_t rtd2Val();


void main(void)
{
	//Initialize LED
	ledInit();
					
	//Initialize SPI & SD

	SPI_MasterInit();
	fat_init();
	crankClock();
	CS2up();
		
	while(1)
	{
		volatile int n[1024]; /* n is an array of 10 integers */
		volatile int arrSize = sizeof(n)/sizeof(n[0]);
		
		for(int i=0 ; i<arrSize ; i+=4){
			DATA_WSM  = readWSM(); ///////////////// NEED TO INITIALIZE ALL OF THE ABOVE STUFF (CS2, CRANK, ETC) WHEN READING BOTH WSM AND BCM
			DATA_BCM  = readBCM();
			DATA_RTD1 = rtd1Val();
			DATA_RTD2 = rtd2Val();

			n[i] = DATA_RTD1;
			n[i+1] = DATA_RTD2;
			n[i+2] = DATA_BCM;
			n[i+3] = DATA_WSM;
		}

		errCode = f_write(&file, n, arrSize, &bytesRead); // Will attempt to write string 'helloworld' to file (data.txt)
		errCode = f_close(&file);
		
	}
}

void mainOG(void)
{
	
	//Initialize LED
	ledInit();
	
	//Initialize SPI & SD

	SPI_MasterInit();
	//fat_init();		
	crankClock();
	CS2up();
	
	while(1)
	{
		set_PORTD_bit(6,0);
		volatile int n[512]; /* n is an array of 10 integers */
		volatile int arrSize = sizeof(n)/sizeof(n[0]);
		
		for(int i=0 ; i<arrSize ; i+=3){
			//DATA_WSM  = readWSM(); ///////////////// NEED TO INITIALIZE ALL OF THE ABOVE STUFF (CS2, CRANK, ETC) WHEN READING BOTH WSM AND BCM
			DATA_RTD1 = rtd1Val();
			_delay_ms(1000);
			DATA_BCM  = readBCM();
			afeTurnOff();
			
			//n[i] = DATA_RTD1;
			n[i+1] = DATA_BCM;
			//n[i+2] = DATA_WSM;
		}

		//errCode = f_write(&file, n, arrSize, &bytesRead); // Will attempt to write string 'helloworld' to file (data.txt)
		//errCode = f_close(&file);
		set_PORTD_bit(6,1);
		
	}
}



//SPI Functions-------------------------------------------------------------

void SPI_MasterInit(void){
	//Function modified from ATmega 1284P Datasheet
	/* Set MOSI and SCK output, all others input */
	DDRB |= (1<<CS2)|(1<<CS1)|(1<<MOSI)|(1<<SCK);
	
	/* Enable SPI, Master, set clock rate fck/4 -> 250kHz */
	SPCR = (1<<SPE)|(1<<MSTR);
	
	// set clock rate fck/64 (SD Initialization)(Crank up After)
	SPCR = SPCR|(0<<SPR0)|(1<<SPR1);
	SPSR = SPSR|(0<<SPI2X);
}

void SPI_MasterTransmit(unsigned char cData){
	//Function copied from ATmega 1284P Datasheet
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

uint8_t SPI_MasterReceive(unsigned char cdata){
	SPDR=cdata;
	while(!(SPSR & (1<<SPIF))){
		//do nothing
	}
	/* Return Data Register */
	return SPDR;
}

void crankClock(){
	// Call after sdInit()
	// crank up clock rate fck/4
	SPCR &= 0b11111100;
	SPSR &= 0b11111110;
}

void CS1up(){
	//SDCARD
	PORTB |= (1<<CS1); //Assert PB2 HIGH
}

void CS1down(){
	//SDCARD
	PORTB &= ~(1<<CS1); //Assert PB2 LOW
}

void CS2up(){
	//AFE4300
	PORTB |= (1<<CS2); //Assert PB2 HIGH
}

void CS2down(){
	//AFE4300
	PORTB &= ~(1<<CS2); //Assert PB2 LOW
}

//AFE Functions-------------------------------------------------------------

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
	SPI_MasterTransmit(0x00); // Avg current Output = 300uA
	SPI_MasterTransmit(0x0F); // 14kHz, Good up to 64kHz (0x41)

	
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
	//SPI_MasterTransmit(0xC9); // Single Ended ADC Meas Mode
	SPI_MasterTransmit(0xC1); // DIFF MODE 
	SPI_MasterTransmit(0xB0); // 64 SPS,
	
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
	/*
	SPI_MasterTransmit(0x0A); // ISW Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0B); // VSense Mux
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x0C); // IQ Mode Enable
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	*/
	
	SPI_MasterTransmit(0x0D); // Weight Scale Control
	SPI_MasterTransmit(0x60); //00
	SPI_MasterTransmit(0x00); // 2nd Stage Gain = 1, No Offset DAC Val
	/*
	SPI_MasterTransmit(0x0E); // BCM DAC Freq
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	*/
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
	SPI_MasterTransmit(0xC1); //?
	SPI_MasterTransmit(0xD0); //250 SPS
	
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

void afeTurnOff(){
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
	SPI_MasterTransmit(0x00);
	/*
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
	SPI_MasterTransmit(0x00); // Avg current Output = 300uA
	SPI_MasterTransmit(0x0F); // 14kHz, Good up to 64kHz (0x41)

	
	SPI_MasterTransmit(0x0F); // Dev Cont 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x00);
	
	SPI_MasterTransmit(0x10); // ADC Control Reg 2
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x63);
	*/
	SPI_MasterTransmit(0x1A); // Misc Reg 3
	SPI_MasterTransmit(0x00);
	SPI_MasterTransmit(0x30);
	//Config Code END
	
	//Begin ADC Conversion
	SPI_MasterTransmit(0x01); // ADC Control Reg 1
	SPI_MasterTransmit(0x41); 
	SPI_MasterTransmit(0xC0); 
	
	CS2up();
}

//LED Functions-------------------------------------------------------------

void ledInit(){
	DDRD = 0b01100000;  // Enabling PD5&PD6
}

int set_PORTD_bit(int position, int value)
{
	// Sets or clears the bit in position 'position'
	// either high or low (1 or 0) to match 'value'.
	// Leaves all other bits in PORTB unchanged.
	
	//Positions Used (5,6)
	
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

//RTD Functions-------------------------------------------------------------

void rtd1Init(){
	ADMUX = 0x01; //enables ADC1 (RTD1)
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC // Start A2D Conversions // ADC frequency @ 125kHz sample rate for 1Mhz clock (x8 Division Factor) 
}

uint16_t rtd1Val(){
	//outputs 10bit result [ADCH,ADCL]
	rtd1Init();
	adcl_val=ADCL;
	adch_val=ADCH;
	adcvalComb = adch_val << 8 | adcl_val;
	return adcvalComb;
}

void rtd2Init(){
	ADMUX = 0x00; //enables ADC0 (RTD2)
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC // Start A2D Conversions // ADC frequency @ 125kHz sample rate for 1Mhz clock (x8 Division Factor)
}

uint16_t rtd2Val(){
	//outputs 10bit result [ADCH,ADCL]
	rtd2Init();
	adcl_val2=ADCL;
	adch_val2=ADCH;
	adcvalComb2 = adch_val2 << 8 | adcl_val2;
	return adcvalComb2;
}

//SD Functions-------------------------------------------------------------

void fat_init(void){
    errCode = -1;
	
	//go until f_open returns FR_OK (function successful)
    while (errCode != FR_OK){                               
        errCode = f_mount(0, &fatfs);                       //mount drive number 0
        errCode = f_opendir(&dir, "/");				    	//root directory

        errCode = f_open(&file, "/hw1256.txt", FA_CREATE_ALWAYS | FA_WRITE);
        if(errCode != FR_OK){
            result=0; //used as a debugging flag
		}                                       
    }
}
