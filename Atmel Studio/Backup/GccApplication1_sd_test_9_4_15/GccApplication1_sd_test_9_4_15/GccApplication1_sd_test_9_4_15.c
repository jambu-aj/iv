/*
 * GccApplication1_sd_test_9_4_15.c
 *
 * Created: 9/4/2015 10:48:20 AM
 *  Author: andyjambu
 */ 


#include <avr/io.h>
#include "ff.h"
#include "diskio.h"

int main(void)
{
    while(1)
    {
        //TODO:: Please write your application code 
		FRESULT f_err_code;
		static FATFS FATFS_Obj;
		
		disk_initialize(0);
		f_err_code = f_mount(0, &FATFS_Obj);
    }
}