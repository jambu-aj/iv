/*******************************************************************************
 *
 *  HAL_SDCard.c - Driver for the SD Card slot
 *
 *  Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/***************************************************************************//**
 * @file       HAL_SDCard.c
 * @addtogroup HAL_SDCard
 * @{
 ******************************************************************************/
#include "msp430.h"
#include "HAL_SDCard.h"

// Pins from MSP430 connected to the SD Card
/*
#define SPI_SIMO        BIT0  // 0x0001
#define SPI_SOMI        BIT1  // 0x0002
#define SPI_CLK         BIT2  // 0x0004
#define SD_CS           BIT5  // 0x0020
*/
#define SPI_SIMO        BIT6  // 0x0001
#define SPI_SOMI        BIT7  // 0x0002
#define SPI_CLK         BIT5  // 0x0004
#define SD_CS           BIT4  // 0x0020
// Ports
#define SPI_SEL         P1SEL
#define SPI_DIR         P1DIR
#define SPI_OUT         P1OUT
#define SPI_REN         P1REN
#define SD_CS_SEL       P1SEL
#define SD_CS_OUT       P1OUT
#define SD_CS_DIR       P1DIR

/***************************************************************************//**
 * @brief   Initialize SD Card
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_init(void)
{
	//*
    // Port initialization for SD Card operation
    SPI_SEL |= SPI_CLK + SPI_SOMI + SPI_SIMO;			   //Set P3.0-2 to peripherals
    SPI_DIR |= SPI_CLK + SPI_SIMO;						   //THIS MIGHT BE UNNECESSARY****
    SPI_REN |= SPI_SOMI;                                   // Pull-Ups on SD Card SOMI
    SPI_OUT |= SPI_SOMI;                                   // Certain SD Card Brands need pull-ups

    SD_CS_SEL &= ~SD_CS;
    SD_CS_OUT |= SD_CS;
    SD_CS_DIR |= SD_CS;

    // Initialize USCI_B1 for SPI Master operation
    UCB0CTL1 |= UCSWRST;                                   // Put state machine in reset
    UCB0CTL0 = UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC; // 3-pin, 8-bit SPI master
    // Clock polarity select - The inactive state is high
    // MSB first
    UCB0CTL1 = UCSWRST + UCSSEL_2;                         // Use SMCLK, keep RESET
    UCB0BR0 = 63;                                          // Initial SPI clock must be <400kHz
    UCB0BR1 = 0;                                           // f_UCxCLK = 25MHz/63 = 397kHz
    UCB0CTL1 &= ~UCSWRST;                                  // Release USCI state machine
    UCB0IFG &= ~UCRXIFG;
}

/***************************************************************************//**
 * @brief   Enable fast SD Card SPI transfers. This function is typically
 *          called after the initial SD Card setup is done to maximize
 *          transfer speed.
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_fastMode(void)
{
    UCB0CTL1 |= UCSWRST;                                   // Put state machine in reset
    UCB0BR0 = 2;                                           // f_UCxCLK = 25MHz/2 = 12.5MHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                                  // Release USCI state machine
}

/***************************************************************************//**
 * @brief   Read a frame of bytes via SPI
 * @param   pBuffer Place to store the received bytes
 * @param   size Indicator of how many bytes to receive
 * @return  None
 ******************************************************************************/

void SDCard_readFrame(uint8_t *pBuffer, uint16_t size)
{
    uint16_t gie = __get_SR_register() & GIE;              // Store current GIE state

    __disable_interrupt();                                 // Make this operation atomic

    UCB0IFG &= ~UCRXIFG;                                   // Ensure RXIFG is clear

    // Clock the actual data transfer and receive the bytes
    while (size--){
        while (!(UCB0IFG & UCTXIFG)) ;                     // Wait while not ready for TX
        UCB0TXBUF = 0xff;                                  // Write dummy byte
        while (!(UCB0IFG & UCRXIFG)) ;                     // Wait for RX buffer (full)
        *pBuffer++ = UCB0RXBUF;
    }

    __bis_SR_register(gie);                                // Restore original GIE state
}

/***************************************************************************//**
 * @brief   Send a frame of bytes via SPI
 * @param   pBuffer Place that holds the bytes to send
 * @param   size Indicator of how many bytes to send
 * @return  None
 ******************************************************************************/

void SDCard_sendFrame(uint8_t *pBuffer, uint16_t size)
{
    uint16_t gie = __get_SR_register() & GIE;              // Store current GIE state

    __disable_interrupt();                                 // Make this operation atomic

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (size--){
        while (!(UCB0IFG & UCTXIFG)) ;                     // Wait while not ready for TX
        UCB0TXBUF = *pBuffer++;                            // Write byte
    }
    while (UCB0STAT & UCBUSY) ;                            // Wait for all TX/RX to finish

    UCB0RXBUF;                                             // Dummy read to empty RX buffer
                                                           // and clear any overrun conditions

    __bis_SR_register(gie);                                // Restore original GIE state
}

/***************************************************************************//**
 * @brief   Set the SD Card's chip-select signal to high
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_setCSHigh(void)
{
    SD_CS_OUT |= SD_CS;
}

/***************************************************************************//**
 * @brief   Set the SD Card's chip-select signal to low
 * @param   None
 * @return  None
 ******************************************************************************/

void SDCard_setCSLow(void)
{
    SD_CS_OUT &= ~SD_CS;
}

/***************************************************************************//**
 * @}
 ******************************************************************************/