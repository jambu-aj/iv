/*
Created to Enable SPI Read Write For IV Project
 */ 

/*
Necessary Include Files
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
*/

/*
Necessary Variable Definitions
unsigned char r1;
unsigned char r7;
unsigned char r2;
*/

/*
Necessary Function Prototypes
unsigned char sendCmd(unsigned char address ,unsigned long arg, unsigned char crc);//proto
bool sdRead(unsigned char * DATA, unsigned long address);
bool sd_mmc_spi_wait_not_busy(void);
*/

#define CS_SD 0x04

int mainFunction(void)
{
    while(1)
    {
	
        //TODO:: Please write your application code
		CSdown(); 
		Init_SPISystem();
		
		//Check if Write Able to Transfer Data
		unsigned char toSend[512];
		toSend[0]=0xAA; //Test Byte
		for (unsigned long i=0;i<2000;i+=512){ //ORIGINALLY 1074000000	
			sdWrite(toSend,i);	
		}	
		
		//Check if Read Confirms Data
		/*	
		if(sd_mmc_spi_wait_not_busy()==true){
				unsigned char toReceive[512];
				sdRead(toReceive,0);
				CSup_SD();
		}		
		*/
		
    }
}

void Init_SPISystem(void)
{
	/* Set MOSI and SCK and CS output, all others input */
	DDRB |= (1<<PORTB5)|(1<<PORTB7)|(1<<CS_SD);
	/* Enable SPI, Master */
	SPCR = (1<<SPE)|(1<<MSTR);

	// set clock rate fck/64
	SPCR = SPCR|(0<<SPR0)|(1<<SPR1);
	SPSR = SPSR|(0<<SPI2X);

	sdInit(); //-< ENABLE WHEN SD IS READY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//delay(2000);

	// crank up clock rate fck/4
	SPCR &= 0b11111100;
	SPSR &= 0b11111110;
	//SPCR |= (0<<SPR0)|(0<<SPR1);
	//SPSR |= (0<<SPI2X);

}

void SPITransmit(unsigned char data){
	SPDR=data;
	while(!(SPSR & (1<<SPIF))){
		//do nothing
	}
}

unsigned char SPIReceive(){
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)));
	/* Return Data Register */
	return SPDR;
}

unsigned char SPITransceive(unsigned char data){
	SPITransmit(data);
	return SPIReceive();
}

void CSup_SD(){// chip select line UP
	PORTB|=(1<<CS_SD);
}

void CSdown(){// chip select line DOWN
	PORTB &= ~(1<<CS_SD);
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

void sdInit(){
	wait_one_second();
	CSup_SD();
	for(char i=0;i<10;i++){
		SPITransmit(0xff);
	}	
	CSdown();
	
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
	CSup_SD();
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
	
	CSdown();
	r1=0x01;
	//r2=0;
	while(r1!=0x00){ //...................................................doesnt work
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
		tester = SPITransceive(0xff); //look @ SPDR
		if(tester == 0xE5 ){//If data accepted
				
				CSup_SD();
				break;
		} else if(tester == 0x0b){ // If CRC error
			//LEDTestY(); // yellow LED error signal	
		} else if(tester == 0x0d){ // If Write error
			//LEDTestG(); // green LED error signal		
		}
	}			
		
}

bool sdRead(unsigned char * DATA, unsigned long address){
		CSdown();
		r1=0x01;
		r2=0;
		r1=sendCmd(0x11,address,0xff);// CMD17 ................. All write commands work, just cant read....
			
		while(SPITransceive(0xff)!=0xfe){//do nothing
		}


		for(int i=0;i<512;i++){//512 is the default block size for SDv1
			DATA[i]=SPITransceive(0xff);	
		}
	
		SPITransceive(0xff);
		SPITransceive(0xff);
		CSup_SD();
}


bool sd_mmc_spi_wait_not_busy(void){
  uint32_t retry;

  // Select the SD_MMC memory gl_ptr_mem points to
  CSdown();
  retry = 0;
  r1=0x00;
  while( r1 != 0xFF){
	r1 = SPITransceive(0xFF);
    retry++;
    if (retry == 200000){
      CSup_SD();
      return false;
    }
  }
  CSup_SD();
  return true;
}

