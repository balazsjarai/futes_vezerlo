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

#ifndef SPI_H
#define SPI_H

#define SPI_CS_PORT PORTA
#define SPI_CS_DDR	DDRA

#define SPI_PORT 	PORTB
#define SPI_DDR 	DDRB

//PINS
#define SCK_POS 	PINB1
#define MISO_POS 	PINB3
#define MOSI_POS	PINB2
#define SS_POS		PINB0

void SPIInit();
void SPIClose();

uint8_t SPIWrite(uint8_t address, uint8_t data);

#endif
