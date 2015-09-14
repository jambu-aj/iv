#include <MSP430F5529.h>
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <math.h>

volatile unsigned int i,ii,iii,results[5];           // Needs to be global in this example
                                            // Otherwise, the compiler removes it
                                            // because it is not used for anything.
volatile unsigned int adc[14]={0},adc1[14]={0},adc2[14]={0},adc3[14]={0},adc4[14]={0};
volatile unsigned int adc_avg,adc1_avg,adc2_avg,adc3_avg,adc4_avg;

int idc,qdc,iac,qac;
long ibody=1100,ibody_range=10,ibody_h,ibody_l,ibody_ms;
long o1,o2;
long ms_dur=120000;   // 60 sec of data at 2 kHz
long iiii;
char str0[100];
char str1[100];
char str2[100];
char str3[100];

unsigned int bytesWritten;
void initCLK(void);
void SetVcoreUp (unsigned int level);
FRESULT WriteFile(char*, char*, WORD);
void fat_init(void);

FIL file;                                               /* Opened file object */
FATFS fatfs;                                            /* File system object */
DIRS dir;                                               /* Directory object   */
FRESULT errCode;                                        /* Error code object  */
FRESULT res;                                            /* Result object      */
UINT bytesRead;                                         /* Bytes read object  */
UINT read;                                              /* Read bytes object  */

unsigned char MST_Data,SLV_Data;
BYTE buffer[32];
int result=1,cnt=0,cnt1=0;

int mr_d,cr_d,mx_d,cx_d,mr_fd,cr_fd,mx_fd,cx_fd;
long sum_Q2,sum_Q3,sum_Q4,sum_Q1;

const float x2=0,x4=-46.8103,x3=-33.8628,x1=-67.7255,x5=-40.81,x6=-37.89,x7=-61.21;
const int delayy=20;

int adc_Avg13(int num1);                             // Function averaging the ADC12 output - 13 samples
int adc_2(int num1);                             // Function averaging the ADC12 output - 2 samples

	void main(void)
{
    WDTCTL = WDTPW+WDTHOLD;                     // Stop watchdog timer

    P1DIR |= BIT5;                            // P1.5 => U/D' of 1k POT
    P2DIR |= BIT2;                            // P2.2 => CS' of 1k POT
    P2DIR |= BIT4;                            // P2.4 => INC of 1k POT
    P2DIR |= BIT0;                            // P2.0 => U/D' of 5k POT
    P2DIR |= BIT5;                            // P2.5 => CS' of 5k POT

    P4DIR |= BIT0;                            // P4.0 set as the select bit S0 of 8-by-1 MUX
    P3DIR |= BIT7;                            // P3.7 set as the select bit S1 of 8-by-1 MUX
    P8DIR |= BIT2;                            // P8.2 set as the select bit S2 of 8-by-1 MUX
    P2DIR |= BIT3;                            // P2.3 set as the select bit S3 of both 2-by-1 MUXs


    P6DIR |= BIT5;                            // P6.5 set as output to observe the frequency at which the SD card is written
    P6OUT &= ~BIT5;                          // XOR P6.5 to observe how long one correction and writing of the SD Card

    P1DIR |= BIT0;                            // Red LED
    P4DIR |= BIT7;                            // Green LED

    P1OUT&= ~BIT0;                          // Red LED
    P4OUT&= ~BIT7;                          // Green LED

    initCLK();


    P6SEL = 0x1F;                             // Enable A/D channel inputs (AD0, AD1, AD2, AD3, AD4)

    REFCTL0 &= ~REFMSTR;                      // Reset REFMSTR to hand over control to
                                              // ADC12_A ref control registers

    ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_12+ADC12REFON+ADC12REF2_5V ; // Turn on ADC12, set sampling time
    ADC12CTL1 = ADC12SSEL_0+ADC12SHP+ADC12CONSEQ_3;       // Use sampling timer, repeat-sequence-of-channels
    ADC12MCTL0 = ADC12INCH_0+ADC12SREF_5;                 // Vr+=Vref+ and Vr-=AVss, channel = A0
    ADC12MCTL1 = ADC12INCH_1+ADC12SREF_5;                 // Vr+=Vref+ and Vr-=AVss, channel = A1,
    ADC12MCTL2 = ADC12INCH_2+ADC12SREF_5;                 // Vr+=Vref+ and Vr-=AVss, channel = A2
    ADC12MCTL3 = ADC12INCH_3+ADC12SREF_5;                 // Vr+=Vref+ and Vr-=AVss, channel = A3
    ADC12MCTL4 = ADC12INCH_4+ADC12SREF_5+ADC12EOS;        // Vr+=Vref+ and Vr-=AVss, channel = A4, end of sequence
    __delay_cycles(30000);                              // Delay for reference start-up

    ADC12IE = 0x10;                           // Enable ADC12IFG.4
    ADC12CTL0 |= ADC12ENC;                    // Enable conversions

    __delay_cycles(10000);                    // Delay for reference start-up

    __enable_interrupt();

    fat_init();                                 //mount, set directory to read from, assign file

    f_write(&file, "i", 1, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
    f_write(&file, "del_i", 1, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
    f_write(&file, "q", 5, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
    f_write(&file, "del_q", 5, &bytesWritten);
    f_write(&file, "\n", 1, &bytesWritten);


 	idc=adc_Avg13(1);
 	iac=adc_Avg13(2);
 	qdc=adc_Avg13(3);
 	qac=adc_Avg13(4);

    sprintf(str0, "%d", idc);
	  f_write(&file, &str0, 4, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
	  sprintf(str1, "%d", iac);
	  f_write(&file, &str1, 4, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
	  sprintf(str2, "%d", qdc);
	  f_write(&file, &str2, 4, &bytesWritten);
    f_write(&file, ",", 1, &bytesWritten);
	  sprintf(str3, "%d", qac);
	  f_write(&file, &str3, 4, &bytesWritten);
    f_write(&file, "\n", 1, &bytesWritten);



    P1OUT ^= BIT0;                          // XOR P1.0
    __delay_cycles(5000000);                      //
    P1OUT ^= BIT0;                          // XOR P1.0
    __delay_cycles(5000000);                      //
    P1OUT ^= BIT0;                          // XOR P1.0
    __delay_cycles(5000000);                      //
    P1OUT ^= BIT0;                          // XOR P1.0



f_close(&file);

P4OUT ^= BIT7;                          // Green LED
__delay_cycles(5000000);                      //
P1OUT ^= BIT0;                          // Red LED
__delay_cycles(5000000);                      //
 P4OUT ^= BIT7;                          // Green LED
 __delay_cycles(5000000);                      //
 P1OUT ^= BIT0;                          // Red LED
 __delay_cycles(5000000);                      //
 P4OUT ^= BIT7;                          // Green LED
 __delay_cycles(5000000);                      //
 P1OUT ^= BIT0;                          // Red LED
 __delay_cycles(5000000);                      //
 P4OUT ^= BIT7;                          // Green LED
 __delay_cycles(5000000);                      //
 P1OUT ^= BIT0;                          // Red LED
 __delay_cycles(5000000);                      //
 P4OUT ^= BIT7;                          // Green LED
 __delay_cycles(5000000);                      //
 P1OUT ^= BIT0;                          // Red LED
 __delay_cycles(5000000);                      //


  P4OUT&= ~BIT7;                          // Green LED
  P1OUT&= ~BIT0;                          // Red LED


}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6: break;                           // Vector  6:  ADC12IFG0
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;							// Vector 12:  ADC12IFG3
  case 14:                                  // Vector 14:  ADC12IFG4

  results[0] = ADC12MEM0;                   // Move results, IFG is cleared
  results[1] = ADC12MEM1;                   // Move results, IFG is cleared
  results[2] = ADC12MEM2;                   // Move results, IFG is cleared
  results[3] = ADC12MEM3;                   // Move results, IFG is cleared
  results[4] = ADC12MEM4;                   // Move results, IFG is cleared


  __bic_SR_register_on_exit(LPM0_bits);     // Exit active CPU, SET BREAKPOINT HERE

  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
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


void initCLK(void){
  volatile unsigned int i;

  // Increase Vcore setting to level3 to support fsystem=25MHz
  // NOTE: Change core voltage one level at a time..
  SetVcoreUp (0x01);
  SetVcoreUp (0x02);
  SetVcoreUp (0x03);

  UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
  UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

  __bis_SR_register(SCG0);                  // Disable the FLL control loop
  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
  UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
  UCSCTL2 = FLLD_1 + 762;                   // Set DCO Multiplier for 25MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (762 + 1) * 32768 = 25MHz
                                            // Set FLL Div = fDCOCLK/2
  __bic_SR_register(SCG0);                  // Enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
  __delay_cycles(782000);
 // __delay_cycles(7);
  // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                            // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
}

void SetVcoreUp (unsigned int level)
{
  // Open PMM registers for write
  PMMCTL0_H = PMMPW_H;
  // Set SVS/SVM high side new level
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  // Set SVM low side to new level
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  // Wait till SVM is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  // Clear already set flags
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;
  // Wait till new level reached
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  // Set SVS/SVM low side to new level
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  // Lock PMM registers for write access
  PMMCTL0_H = 0x00;
}

int adc_Avg13(int num1)
{
    ADC12CTL0 &= ~ADC12ENC;                    // Stop conversions
    ADC12CTL0 |= ADC12ENC;                    // Enable conversions

    for(iii=0;iii<14;iii++)
	    {
	    ADC12CTL0 |= ADC12SC;                     // Start conversion
		__delay_cycles(50000);
	    adc[iii]=results[0];
	    adc1[iii]=results[1];
	    adc2[iii]=results[2];
	    adc3[iii]=results[3];
	    adc4[iii]=results[4];
	    ADC12CTL0 &= ~ADC12ENC;                    // Stop conversions
	    }
	    adc_avg=(adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]+adc[10]+adc[11]+adc[12]+adc[13])/13;
	    adc1_avg=(adc1[1]+adc1[2]+adc1[3]+adc1[4]+adc1[5]+adc1[6]+adc1[7]+adc1[8]+adc1[9]+adc1[10]+adc1[11]+adc1[12]+adc1[13])/13;
	    adc2_avg=(adc2[1]+adc2[2]+adc2[3]+adc2[4]+adc2[5]+adc2[6]+adc2[7]+adc2[8]+adc2[9]+adc2[10]+adc2[11]+adc2[12]+adc2[13])/13;
	    adc3_avg=(adc3[1]+adc3[2]+adc3[3]+adc3[4]+adc3[5]+adc3[6]+adc3[7]+adc3[8]+adc3[9]+adc3[10]+adc3[11]+adc3[12]+adc3[13])/13;
	    adc4_avg=(adc4[1]+adc4[2]+adc4[3]+adc4[4]+adc4[5]+adc4[6]+adc4[7]+adc4[8]+adc4[9]+adc4[10]+adc4[11]+adc4[12]+adc4[13])/13;

	    if(num1==0)
	    return adc_avg;
	    else if(num1==1)
	  	return adc1_avg;
	    else if(num1==2)
	  	return adc2_avg;
	    else if(num1==3)
	  	return adc3_avg;
	    else if(num1==4)
	  	return adc4_avg;
}


int adc_2(int num1)
{


	    adc[0]=results[0];
	    adc1[0]=results[1];
	    adc2[0]=results[2];
	    adc3[0]=results[3];
	    adc4[0]=results[4];


 adc_avg=adc[0];
 adc1_avg=adc1[0];
 adc2_avg=adc2[0];
 adc3_avg=adc3[0];
 adc4_avg=adc4[0];


	    if(num1==0)
	    return adc_avg;
	    else if(num1==1)
	  	return adc1_avg;
	    else if(num1==2)
	  	return adc2_avg;
	    else if(num1==3)
	  	return adc3_avg;
	    else if(num1==4)
	  	return adc4_avg;
}

