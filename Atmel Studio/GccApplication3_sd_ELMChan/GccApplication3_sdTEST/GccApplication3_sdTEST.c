#include <avr/io.h>
#include "ff.h"
#include "integer.h"
#include "diskio.h"

FRESULT f_err_code;
static FATFS FATFS_Obj;
FIL fil_obj;
uint16_t toSend = 256;
int check =0;

int main(void)
{

	
	DDRD = 0b01100000;  // Enabling PD5&PD6
	
	PORTD = 0b01100000;
	
	disk_initialize(0);
	f_err_code = f_mount(0, &FATFS_Obj);
	
	if (f_err_code == FR_OK)
	{
		f_err_code = f_open(&fil_obj, "check.txt", FA_WRITE);
		if (f_err_code == FR_OK)
		{
			//f_err_code=f_write(&fil_obj,&toSend,64,&check ) ;
		}
	}
	
	
	
	
	f_close(&fil_obj);
	
	while(1)
	{
		//TODO:: Please write your application code
	}
}
