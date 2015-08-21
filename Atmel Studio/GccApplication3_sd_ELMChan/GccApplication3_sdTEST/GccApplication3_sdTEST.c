/*
 * GccApplication3_sdTEST.c
 *
 * Created: 5/7/2015 11:17:56 AM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <math.h>

volatile unsigned int results[7];           // Needs to be global in this example
// Otherwise, the compiler removes it
// because it is not used for anything.

unsigned char *PRxData,*PDataRec;                     // Pointer to RX data
unsigned char RXByteCtr;
volatile unsigned char DataRec;       // Allocate 128 byte of RAM

unsigned int bytesWritten;
void initCLK(void);
void SetVcoreUp (unsigned int level);
FRESULT WriteFile(char*, char*, WORD);
void fat_init(void); 
int f_putc (TCHAR, FIL*);                                                               /* Put a character to the file */
int f_puts (const TCHAR*, FIL*);                                        /* Put a string to the file */

FIL file;                                               /* Opened file object */
FATFS fatfs;                                            /* File system object */
DIRS dir;                                               /* Directory object   */
FRESULT errCode;                                        /* Error code object  */
FRESULT res;                                            /* Result object      */
UINT bytesRead;                                         /* Bytes read object  */
UINT read;                                              /* Read bytes object  */

unsigned char MST_Data,SLV_Data;
BYTE buffer[32];
int result=1;

int main(void)
{
    fat_init();
}

void fat_init(void){
	errCode = -1;

	while (errCode != FR_OK){                               //go until f_open returns FR_OK (function successful)
		errCode = f_mount(0, &fatfs);                       //mount drive number 0
		errCode = f_opendir(&dir, "/");                                 //root directory

		errCode = f_open(&file, "/data.txt", FA_CREATE_ALWAYS | FA_WRITE);
		if(errCode != FR_OK)
		result=0;                                       //used as a debugging flag
	}
}