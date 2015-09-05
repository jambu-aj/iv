/*
 * GccApplication1_abcd.c
 *
 * Created: 9/4/2015 11:24:09 AM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <math.h>

void fat_init(void);
                                  /* Put a string to the file */

FIL file;                                               /* Opened file object */
FATFS fatfs;                                            /* File system object */
DIR dir;                                               /* Directory object   */
FRESULT errCode;                                        /* Error code object  */
FRESULT res;                                            /* Result object      */
UINT bytesRead;                                         /* Bytes read object  */
UINT read;                                              /* Read bytes object  */

int result=1;



int main(void)
{
	fat_init();
}

void fat_init(void){
	
	while (errCode == FR_OK){                               
		//go until f_open returns FR_OK (function successful)
		errCode = f_mount(&fatfs, "", 0);                       //mount drive number 0
		//errCode = f_mkfs("",0,0);
		//errCode = f_opendir(&dir, "/");                                 //root directory

		errCode = f_open(&file, "/data.txt", FA_CREATE_ALWAYS | FA_WRITE);
		if(errCode != FR_OK){
			result=0;                                       //used as a debugging flag
			break;
		}
	}
}	

void fat_init2(void){
	
    FIL fil;       /* File object */
    char line[82]; /* Line buffer */
    FRESULT fr;    /* FatFs return code */


    /* Register work area to the default drive */
    f_mount(&fatfs, "", 0);

    /* Open a text file */
    fr = f_open(&fil, "message.txt", FA_READ);
    if (fr) return (int)fr;

    /* Read all lines and display it */
    while (f_gets(line, sizeof line, &fil))
    printf(line);

    /* Close the file */
    f_close(&fil);

    return 0;
}
DWORD get_fattime (void)
{
	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(2013 - 1980) << 25)
	| ((DWORD)3 << 21)
	| ((DWORD)23 << 16)
	| ((DWORD)12 << 11)
	| ((DWORD)0 << 5)
	| ((DWORD)0 >> 1);
}
