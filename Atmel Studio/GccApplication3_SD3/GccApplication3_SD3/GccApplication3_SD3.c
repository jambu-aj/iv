/*
 * GccApplication3_SD3.c
 *
 * Created: 2/26/2015 12:18:56 AM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define CS_SD PORTB4
#define CS_AFE PORTB2

unsigned char r1, r2, r7; // SD R/W Flags
unsigned char msb;
unsigned char lsb;
uint16_t real;

unsigned char sendCmd(unsigned char address ,unsigned long arg, unsigned char crc);//Function Prototype only
bool sdRead(unsigned char * DATA, unsigned long address);//Function Prototype only
bool sd_mmc_spi_wait_not_busy(void);//Function Prototype only
uint16_t AFEReceive();//Function Prototype only
unsigned char SPIReceive();//Function Prototype only
unsigned char SPITransceive(unsigned char data);//Function Prototype only

int main(void)
{
    while(1)
    {
		sdTest();
    }
}

/*
************************************************************************************************************
TEST FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

void sdTest(){
		
		//TODO:: Please write your application code
		CS_SD_Down();
		Init_SPISystem();
		
		//Check if Write Able to Transfer Data
		unsigned char toSend[512];
		toSend[0]=0xAA; //Test Byte
		for (unsigned long i=0;i<2000;i+=512){ 
			//ORIGINALLY 1074000000
		sdWrite(toSend,i);
	}
	
	//Check if Read Confirms Data
	
	if(sd_mmc_spi_wait_not_busy()==true){
		unsigned char toReceive[512];
		sdRead(toReceive,0);
		CS_SD_Up();
	}
}

void afeTest(){
		/*
		DDRB = 0b00001000;  // Enabling PB3 (LED)
		PORTB |= (1 << 3);  // Set bit # 'position' high (LED)
		*/
		Init_SPISystem(); //Set up SPI Protocol
		//Init_AFE_BCM();		

			AFETransmit(0x014140); //SPITransmit(0x010140);//ADC Control Register 1
			AFETransmit(0x020000);//Misc Reg 1
			AFETransmit(0x03FFFF);//Misc Reg 2
			AFETransmit(0x096006);//Device Control 1
			AFETransmit(0x0A0804);//ISW_Mux
			AFETransmit(0x0B0804);//VSENSE_Mux
			AFETransmit(0x0E0001);//BCM_DAC_FREQ
			AFETransmit(0x100063);//ADC Control Register 2
			AFETransmit(0x1A0030);//Misc Reg 3
			

	//	msb=0x12;
	//	lsb=0x13;
		while(1){
			CS_AFE_Down();
			SPITranceive(0x20);
			SPITranceive(0x00);
			SPITranceive(0x00);
			/*
			SPITransmit(0x20);//Read address
			msb=SPIReceive();
			SPITransmit(0x00);//MSB plz
			lsb=SPIReceive();
			SPITransmit(0x00);//LSB plz
			lsb=SPIReceive();
			*/
			CS_AFE_Up();
			//real=msb|(lsb>>8);
		}
}

void rtdTest(){
	
	uint8_t adcVal=0;
	Init_LED();
	
	for (;;){
		adcVal=ADCH;
		if(adcVal > 128){
			set_PORTD_bit(3, 1);    // Set PB3 high
			ADCSRA |= (1 << ADSC); // Required before reading a single ADCH value
		}
		else{
			set_PORTD_bit(3, 0);    // Set PB3 low
			ADCSRA |= (1 << ADSC); // Start A2D Conversions
		}
	}
}


/*
************************************************************************************************************
SPI FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

void Init_SPISystem(void)
{
	/* Set MOSI and SCK and CS output, all others input */
	DDRB |= (1<<PORTB5)|(1<<PORTB7)|(1<<CS_SD)|(1<<CS_AFE);
	/* Enable SPI, Master */
	SPCR = (1<<SPE)|(1<<MSTR);

	// set clock rate fck/64
	SPCR = SPCR|(0<<SPR0)|(1<<SPR1);
	SPSR = SPSR|(0<<SPI2X);

	sdInit(); //-< ENABLE WHEN SD IS READY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// crank up clock rate fck/4
	SPCR &= 0b11111100;
	SPSR &= 0b11111110;
	
}

void SPITransmit(unsigned char data){
	SPDR=data;
	while(!(SPSR & (1<<SPIF))){
		//do nothing
	}
}

unsigned char SPITransceive(unsigned char data){
		SPDR=data;
		while(!(SPSR & (1<<SPIF))){
			//do nothing
		}
	/* Return Data Register */
	return SPDR;
}

void CS_SD_Up(){
	// chip select line UP
	PORTB|=(1<<CS_SD);
}

void CS_SD_Down(){
	// chip select line DOWN
	PORTB &= ~(1<<CS_SD);
}

void CS_AFE_Up(){
	// chip select line UP
	PORTB|=(1<<CS_AFE);
}

void CS_AFE_Down(){
	// chip select line DOWN
	PORTB &= ~(1<<CS_AFE);
}

void AFETransmit(unsigned long data){ 
	//Sends 3 Byte (24Bit) value as 3 separate 8 bit Char's to SPITransmit() Fn.
	CS_AFE_Down();
	SPITransmit(data);//1st byte
	SPITransmit(data<<8);//2nd byte
	SPITransmit(data<<16);//3rd byte
	msb=SPITransceive(0xff);
	CS_AFE_Up();
}

uint16_t AFEReceive(){
	uint16_t data;
	
	CS_AFE_Down();
	data=SPIReceive();
	data|=(SPIReceive()>>8);
	CS_AFE_Up();
}

void Init_AFE_BCM(void)
{
	AFETransmit(0x010140); //SPITransmit(0x010140);//ADC Control Register 1
	AFETransmit(0x020000);//Misc Reg 1
	AFETransmit(0x03FFFF);//Misc Reg 2
	AFETransmit(0x096005);//Device Control 1
	AFETransmit(0x0A0804);//ISW_Mux
	AFETransmit(0x0B0804);//VSENSE_Mux
	AFETransmit(0x0E0001);//BCM_DAC_FREQ
	AFETransmit(0x100063);//ADC Control Register 2
	AFETransmit(0x1A0030);//Misc Reg 3
}

/*
************************************************************************************************************
LED FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

void Init_LED(){
		// Enabling PD7/PD6 LED
		DDRB |= (1<<PORTD5)|(1<<PORTD6);
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
	
	if (value == 0){
		PORTD &= ~(1 << position);      // Set bit # 'position' low
	}
	else{
		PORTD |= (1 << position);       // Set bit # 'position' high
	}
	return 1;
}

/*
************************************************************************************************************
SD FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

void sdInit(){
	wait_one_second();
	CS_SD_Up();
	for(char i=0;i<10;i++){
		SPITransmit(0xff);
	}	
	CS_SD_Down();
	
	SPITransmit(0xff);
	SPITransmit(0xff);
	
	while(r1!=0x01){
		sendCmd(0,0x00000000,0x95);
	}
	while((r7!=0xAA)){
		sendCmd(8,0x000001AA,0x87);
	}
	while(r1!=0x00){
		sendCmd(55,0x40000000,0xff);
		sendCmd(41,0x40000000,0xff);
	}
	
	sendCmd(58,0x00000000,0xff);
	SPITransmit(0xff);
	
	//sendCmd(16,0x00000200,0xff);
	//SPITransmit(0xff);
	
	//sendCmd(10,0x00000000,0xff); //results in r1=0xff
	CS_SD_Up();
	return 1;
}

unsigned char sendCmd(unsigned char address ,unsigned long arg, unsigned char crc){
	SPITransmit(0xff);
	SPITransmit(address|0x40);
	SPITransmit(arg>>24);
	SPITransmit(arg>>16);
	SPITransmit(arg>>8);
	SPITransmit(arg);
	SPITransmit(crc);
	SPITransmit(0xff);
	r1 = SPITransceive(0xff);
	SPITransceive(0xff);
	SPITransceive(0xff);
	SPITransceive(0xff);
	r7=SPITransceive(0xff);
	return r1;
}


void sdWrite(unsigned char *DATA, unsigned long address){
	
	CS_SD_Down();
	r1=0x01;
	//r2=0;
	while(r1!=0x00){ 
		//...................................................doesnt work
		sendCmd(24,address,0xff); //Gets in 41 tries...why?
		//r2++;
	}
	
	SPITransmit(0xfe); //datatoken
	for (int i=0;i<512;i++){
		SPITransmit(DATA[i]);
	}
	SPITransmit(0x95);
	SPITransmit(0x95);
	char tester=0x01;
	for(int i=0; i<10000; i++){
		tester = SPITransceive(0xff); 
		//look @ SPDR
		if(tester == 0xE5 ){
			//If data accepted		
				CS_SD_Up();
				break;
		} else if(tester == 0x0b){ 
			// If CRC error
		} else if(tester == 0x0d){
			 // If Write error		
		}
	}			
		
}

bool sdRead(unsigned char * DATA, unsigned long address){
		CS_SD_Down();
		r1=0x01;
		r2=0;
		r1=sendCmd(0x11,address,0xff);// CMD17 ................. All write commands work, just cant read....
			
		while(SPITransceive(0xff)!=0xfe){
			//do nothing
		}


		for(int i=0;i<512;i++){
			//512 is the default block size for SDv1
			DATA[i]=SPITransceive(0xff);	
		}
	
		SPITransceive(0xff);
		SPITransceive(0xff);
		CS_SD_Up();
}

bool sd_mmc_spi_wait_not_busy(void){
  uint32_t retry;

  // Select the SD_MMC memory gl_ptr_mem points to
  CS_SD_Down();
  retry = 0;
  r1=0x00;
  while( r1 != 0xFF){
	r1 = SPITransceive(0xFF);
    retry++;
    if (retry == 200000){
      CS_SD_Up();
      return false;
    }
  }
  CS_SD_Up();
  return true;
}

/*
************************************************************************************************************
RTD FUNCTIONS-----------------------------------------------------------------------------------------------
************************************************************************************************************
*/

void Init_RTD(){
	//Enabling ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enabling ADC frequency @ 125kHz sample rate for 16Mhz clock
	ADMUX |= (1 << REFS0); // Set ADC reference (AREF) to AVCC
	ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading
	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADSC); // Start A2D Conversions
}