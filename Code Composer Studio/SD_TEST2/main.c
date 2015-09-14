#include <msp430.h> 
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <math.h>

void fat_init(void);

FIL file;                                               /* Opened file object */
FATFS fatfs;                                            /* File system object */
DIRS dir;                                               /* Directory object   */
FRESULT errCode;                                        /* Error code object  */
FRESULT res;                                            /* Result object      */
UINT bytesRead;                                         /* Bytes read object  */
UINT read;


int result=1;
/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}

void fat_init(void){
    errCode = -1;

    while (errCode != FR_OK){                               //go until f_open returns FR_OK (function successful)
        errCode = f_mount(0, &fatfs);                       //mount drive number 0
        errCode = f_opendir(&dir, "/");				    	//root directory

        errCode = f_open(&file, "/data.csv", FA_CREATE_ALWAYS | FA_WRITE);
        if(errCode != FR_OK)
            result=0;                                       //used as a debugging flag
    }
}
