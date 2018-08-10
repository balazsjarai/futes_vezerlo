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

unsigned char SPIWrite(uint8_t address, uint8_t data);

#endif
