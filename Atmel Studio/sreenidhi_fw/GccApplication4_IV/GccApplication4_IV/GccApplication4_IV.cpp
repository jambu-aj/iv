#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "usb_serial.h"


/*Function prototypes*/
void ledInit(void);
void crankClock();
void sdInit();
void rtd1Init();
void rtd2Init();
void SPI_MasterInit(void);
void SPI_MasterTransmit(unsigned char cData);
unsigned char SPI_MasterReceive(unsigned char cdata);
void CS1down();
void CS1up();
void CS2up();
void CS2down();
void AFE_Init_BCM_SS();
void AFE_Init_BCM_Cont();
uint16_t readBCM();
void AFE_Init_WSM_SS();
void AFE_Init_WSM_Cont();
uint16_t readWSM();

bool sd_mmc_spi_wait_not_busy(void);
bool sdRead(unsigned char * DATA, unsigned long address);

uint16_t rtd1Val();
uint16_t rtd2Val();

/*Definitions*/
#define MOSI DDB5
#define SCK DDB7
#define CLK0 DDB1
#define CS2 DDB2
#define CS1 DDB4
#define RDY DDB0

#define LED1 DDD5
#define LED2 DDD6

/*Variables*/

uint16_t DATA_RTD1; //RTD Variables
uint16_t DATA_RTD2;
uint16_t adcvalComb, adcvalComb2;//ADC Combined Values
uint8_t adcl_val, adch_val, adcl_val2, adch_val2;//ADC MSB LSB Values

unsigned char msb; //AFE Variables
unsigned char lsb;
uint16_t msb_lsb_comb;
uint16_t DATA_BCM;
uint16_t DATA_WSM;

unsigned char r1, r2, r7; // SD R/W Flags
char tester;
uint16_t toSend[4];
uint16_t toReceive[4];
uint8_t j=0; //SD Sector #
uint8_t check=0;


int main(void)
{
	
	 FATFS filesystem;
	 FRESULT errCode;
	 
			//Initialize LED
			// LED2: Solid - SD not Present; Blinking - SD Present, proceed to Data Collection
			ledInit();
			
			//Initialize SPI & SD
			CS1down();
			SPI_MasterInit();
			sdInit();
			crankClock();
			CS2up();
			
	while(1){
		

		
		// RTD Main Fn
		rtd1Init();
		DATA_RTD1=rtd1Val();
		rtd2Init();
		DATA_RTD2=rtd2Val();
		
		
		
		//AFE Main Fn

		DATA_BCM=readBCM();
		DATA_WSM=readWSM();
		
		
		//SD Write
		toSend[0]=DATA_RTD1; //Test Byte
		toSend[1]=DATA_RTD2; 
		toSend[2]=DATA_BCM;
		toSend[3]=DATA_WSM;
		 
		//SD Write
		errCode = f_mount(0, &filesystem);
		    if (errCode == FR_OK)
		    {
		
				    DIR rootDir;
					errCode = f_opendir(&rootDir, "/");
					if (errCode == FR_OK)
						{
			 		    
							FILINFO fileEntry;
							errCode = f_readdir(&rootDir, &fileEntry);
							if (errCode == FR_OK)
								{
						    
									// open file
									FIL file;
									errCode = f_open(&file, fileEntry.fname, FA_WRITE);
									if (errCode == FR_OK)
										{
									    
				   							errCode = f_write(&File,*toSend,64,&check ) ;
											
					    
										 }
								  }
						}
						
			}
		    
//for (unsigned long i=0;i<2000;i+=512){
			//ORIGINALLY 1074000000
			//sdWrite(toSend,j); //0x00801000 blank sector beginning (from 1st file test)
		//}
	
		//SD Read (Confirms Data)
		
	//	if(sd_mmc_spi_wait_not_busy()==true){
		//	sdRead(toReceive,j);
			//CS1up();
		//}
		

		
		
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

unsigned char SPI_MasterReceive(unsigned char cdata){
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

//SD Functions-------------------------------------------------------------

void CS1up(){
	PORTB |= (1<<CS1); //Assert PB2 HIGH
}

void CS1down(){
	PORTB &= ~(1<<CS1); //Assert PB2 LOW
}

unsigned char sendCmd(unsigned char address ,unsigned long arg, unsigned char crc){
	SPI_MasterTransmit(0xff);
	SPI_MasterTransmit(address|0x40);
	SPI_MasterTransmit(arg>>24);
	SPI_MasterTransmit(arg>>16);
	SPI_MasterTransmit(arg>>8);
	SPI_MasterTransmit(arg);
	SPI_MasterTransmit(crc);
	SPI_MasterTransmit(0xff);
	r1 = SPI_MasterReceive(0xff);
	SPI_MasterReceive(0xff);
	SPI_MasterReceive(0xff);
	SPI_MasterReceive(0xff);
	r7=SPI_MasterReceive(0xff);
	return r1;
}

void sdInit(){
	_delay_ms(1000);
	CS1up();
	for(char i=0;i<10;i++){
		SPI_MasterTransmit(0xff);
	}
	CS1down();
	
	SPI_MasterTransmit(0xff);
	SPI_MasterTransmit(0xff);
	
	set_PORTD_bit(6,1); //Turn LED2 (Card Present Status)
	
	while(r1!=0x01){
		sendCmd(0,0x00000000,0x95);
	}
	while((r7!=0xAA)){
		set_PORTD_bit(6,0); //Turn LED2 off if Present Card
		sendCmd(8,0x000001AA,0x87);
	}
	while(r1!=0x00){
		sendCmd(55,0x40000000,0xff);
		sendCmd(41,0x40000000,0xff);
	}
	
	sendCmd(58,0x00000000,0xff);
	SPI_MasterTransmit(0xff);
	
	//sendCmd(16,0x00000200,0xff);
	//SPITransmit(0xff);
	
	//sendCmd(10,0x00000000,0xff); //results in r1=0xff
	CS1up();
	//return 1;
}

void sdWrite(unsigned char *DATA, unsigned long address){
	
	CS1down();
	r1=0x01;
	//r2=0;
	while(r1!=0x00){
		sendCmd(24,address,0xff); //Gets in 41 tries...why?
		//r2++;
	}
	
	SPI_MasterTransmit(0xfe); //datatoken
	for (int i=0;i<512;i++){
		SPI_MasterTransmit(DATA[i]);
	}
	SPI_MasterTransmit(0x95);
	SPI_MasterTransmit(0x95);
	tester=0x01;
	for(int i=0; i<10000; i++){
		tester = SPI_MasterReceive(0xff);
		//look @ SPDR
		if(tester == 0xE5 ){
			//If data accepted
			CS1up();
			break;
		} else if(tester == 0x0b){
			// If CRC error
		} else if(tester == 0x0d){
			// If Write error
		}
	}
	
}

bool sdRead(unsigned char *DATA, unsigned long address){
	CS1down();
	r1=0x01;
	r2=0;
	r1=sendCmd(0x11,address,0xff);// CMD17 ................. All write commands work, just cant read....
	
	while(SPI_MasterReceive(0xff)!=0xfe){
		//do nothing
	}


	for(int i=0;i<512;i++){
		//512 is the default block size for SDv1
		DATA[i]=SPI_MasterReceive(0xff);
	}
	
	SPI_MasterReceive(0xff);
	SPI_MasterReceive(0xff);
	CS1up();
}

bool sd_mmc_spi_wait_not_busy(void){
	uint32_t retry;

	// Select the SD_MMC memory gl_ptr_mem points to
	CS1down();
	retry = 0;
	r1=0x00;
	while( r1 != 0xFF){
		r1 = SPI_MasterReceive(0xFF);
		retry++;
		if (retry == 200000){
			CS1up();
			return false;
		}
	}
	CS1up();
	return true;
}


//AFE Functions-------------------------------------------------------------

void CS2up(){
	PORTB |= (1<<CS2); //Assert PB2 HIGH
}

void CS2down(){
	PORTB &= ~(1<<CS2); //Assert PB2 LOW
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
	adcl_val=ADCL;
	adch_val=ADCH;
	adcvalComb |= adch_val << 8 | adcl_val;
	ADCSRA |= (1 << ADSC); // Required before reading a single ADCH value
	
	return adcvalComb;
	
}

void rtd2Init(){
	ADMUX = 0x00; //enables ADC0 (RTD2)
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC // Start A2D Conversions // ADC frequency @ 125kHz sample rate for 1Mhz clock (x8 Division Factor)
}

uint16_t rtd2Val(){
	//outputs 10bit result [ADCH,ADCL]
	adcl_val2=ADCL;
	adch_val2=ADCH;
	adcvalComb2 |= adch_val2 << 8 | adcl_val2;
	ADCSRA |= (1 << ADSC); // Required before reading a single ADCH value
	
	return adcvalComb2;
	
}

