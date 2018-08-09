/******************************************************************************
                        
Simple SPI Communication Functions.

                                     NOTICE
									--------
NO PART OF THIS WORK CAN BE COPIED, DISTRIBUTED OR PUBLISHED WITHOUT A
WRITTEN PERMISSION FROM EXTREME ELECTRONICS INDIA. THE LIBRARY, NOR ANY PART
OF IT CAN BE USED IN COMMERCIAL APPLICATIONS. IT IS INTENDED TO BE USED FOR
HOBBY, LEARNING AND EDUCATIONAL PURPOSE ONLY. IF YOU WANT TO USE THEM IN 
COMMERCIAL APPLICATION PLEASE WRITE TO THE AUTHOR.


WRITTEN BY:
AVINASH GUPTA
me@avinashgupta.com

*******************************************************************************/

#include <avr/io.h>

#include "spi.h"

/********************************************************************

Function To Initialize the Internal SPI Hardware

********************************************************************/
void SPIInit()
{
	//Set up SPI I/O Ports
	SPI_DDR|=((1<<MOSI_POS)|(1<<SCK_POS)|(1<<SS_POS));
	
	SPI_PORT|=(1<<SS_POS);

	//Chip Select PIN is initialize as HIGH i.e. Idle
	//CS PIN is always Active Low (inverted)
	CS_DDR|=(1<<CS_POS);
	CS_HIGH();

	/*

	SPI BUS CONFIGURATION
	---------------------
	*Master Mode
	*MSB first
	*CPOL=0
	*CPHA=0
	*Above two implies SPI MODE0
	*SCK Freq = FCPU/16 i.e. 1MHz

	*/

	SPCR|=(1<<SPE)|(1<<MSTR)|(1<<SPR0);
}



/********************************************************************

Function To Disable the Internal SPI Hardware

********************************************************************/
void SPIClose()
{
	SPCR&=(~(1<<SPE));
	SPI_DDR&=(~((1<<MOSI_POS)|(1<<SCK_POS)));
}



/********************************************************************

Function for SPI Transaction.

Arguments:
	uint8_t data : Data to write

Return Value
	uint8_t : Data Read from BUS

********************************************************************/
void SPI_Write(unsigned char address, unsigned char data)
{
	SPI_CS_PORT &= ~(1 << address);
	_delay_us(1);
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
	SPI_CS_PORT |= (1 << address);
	return;
}

