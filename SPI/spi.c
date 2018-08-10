#include <avr/io.h>
#include "spi.h"

void SPIInit()
{
	//Set up SPI I/O Ports
	SPI_DDR |= (1<<MOSI_POS)|(1<<SCK_POS)|(1<<SS_POS);
	
	SPI_PORT |= (1<<SS_POS)|(1 << MISO_POS);

	SPI_CS_PORT |= 0xFF;
	SPI_CS_DDR |= 0xFF;

	SPCR |= (1 << SPE)|(1 << MSTR)|(1 << SPR0)|(1 << SPR1);
}

void SPIClose()
{
	SPCR&=(~(1<<SPE));
	SPI_DDR&=(~((1<<MOSI_POS)|(1<<SCK_POS)));
}

unsigned char SPIWrite(unsigned char address, unsigned char data)
{
	SPI_CS_PORT &= ~(1 << address);
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
	SPI_CS_PORT |= (1 << address);
	return SPDR;
}

