
#include <avr/io.h>
#include "MAX31865.h"
#include "util/delay.h"



float const A = 1.597E-10;
float const B = 2.951E-8;
float const C = 4.784E-6;
float const D = 2.613E-3;




void SPI_MAX31865(void){
	SPCR =
	(1<<SPE) 	|
	(1<<MSTR) 	|
	(1<<CPHA) 	|
	(1<<SPR0) 	|
	(1<<SPR1);
}

void SPI_OFF()
{
	SPCR &= ~(1 << SPE);
}



//int8_t check_MAX31865(int8_t channel)
//{
//
	//PORTE &=~ (1<<PE2);
//
	//_delay_us(80);
	//switch(channel)
	//{
		//case 0:		{
			//if(!(PINA & (1<<PA0))){
				//return 1;
				//break;
			//}
		//}
//
		//case 1:
		//{
			//if(!(PINA & (1<<PA1))){
				//break;
				//return 1;
			//}
//
		//}
//
	//}
//
	//PORTE |= (1<<PE2);
//
	//return 0;
//}



void init_max31865(int channel){

	//DDRA &=~(1<<PA0);
	//DDRA &=~(1<<PA1);

	//PORTA |= (1<<PA0);
	//PORTA |= (1<<PA1);
	//SET_SPI_LS_OUTPUT;
	SET_SPI_MOSI_OUTPUT;
	SET_SPI_SCK_OUTPUT;
	SET_SPI_MISO_INPUT;

	SET_MAX1_CS_OUTPUT;
	//SET_MAX2_CS_OUTPUT;
	//SET_MAX1_DR_INPUT;
	//SET_MAX2_DR_INPUT;
	
	
	
	
	SET_SPI_MOSI_HIGH;
	SET_SPI_SCK_HIGH;
	ENABLE_PULLUP_SPI_MISO;
	//SET_SPI_LS_HIGH;
	SET_MAX1_CS_HIGH;
	//SET_MAX2_CS_HIGH;

	SPI_MAX31865();

	//SET_SPI_LS_LOW;


	
	switch(channel){
		case 0: {
			SET_MAX1_CS_LOW;
			_delay_us(10);
			SPDR=WRITE_CONFIG;
			while(!(SPSR & (1<<SPIF)));
			SPDR=0xD1;
			while(!(SPSR & (1<<SPIF)));
			SET_MAX1_CS_HIGH;
			break;
		}

		case 1: {
			//SET_MAX2_CS_LOW;
			SPDR=WRITE_CONFIG;
			while(!(SPSR & 0x80));
			SPDR=0xC1;
			while(!(SPSR & 0x80));
			//SET_MAX2_CS_HIGH;
			break;
		}
	}
	//SET_SPI_LS_HIGH;
}


int16_t Read_MAX31865(int channel,int16_t referenz){

	int16_t data;
	float resistance;
	int8_t low=0,high=0;

	SPI_MAX31865();
	//SET_SPI_LS_LOW;
	_delay_us(80);
	switch(channel){

		case 0:
		{
			
			SET_MAX1_CS_LOW;
			_delay_us(10);
			SPDR=READ_RTD_MSB;
			while(!(SPSR & (1<<SPIF)));
			SPDR=DUMMYBYTE;
			while(!(SPSR & (1<<SPIF)));
			high=SPDR;
			SET_MAX1_CS_HIGH;


			SET_MAX1_CS_LOW;
			_delay_us(10);
			SPDR=READ_RTD_LSB;
			while(!(SPSR & (1<<SPIF)));
			SPDR=DUMMYBYTE;
			while(!(SPSR & (1<<SPIF)));
			low=SPDR;
			SET_MAX1_CS_HIGH;
			break;
		}

		case 1:
		{

			//SET_MAX2_CS_LOW;
			SPDR=READ_RTD_MSB;
			while(!(SPSR & 0x80));
			SPDR=DUMMYBYTE;
			while(!(SPSR & 0x80));
			high=SPDR;
			//SET_MAX2_CS_HIGH;


			//SET_MAX2_CS_LOW;
			SPDR=READ_RTD_LSB;
			while(!(SPSR & 0x80));
			SPDR=DUMMYBYTE;
			while(!(SPSR & 0x80));
			low=SPDR;
			//SET_MAX2_CS_HIGH;
			
			break;
		}

	}
	_delay_us(80);
	//SET_SPI_LS_HIGH;

	data=high;
	data<<=8;
	data+=low;
	data>>=1;



	resistance = data;
	resistance = resistance *referenz*10;
	resistance = resistance /32768;
	resistance +=0.5;

	return (int16_t)resistance;
}






int16_t calc_temp_pt100(int16_t resistance)
{

	float R=resistance;
	R=R/100;
	float T=0;


	T=(A*(R*R*R*R*R)-B*(R*R*R*R)-C*(R*R*R)+D*(R*R)+2.219*R-241.9);
	T=T*10;
	T=T+0.5;
	return (int16_t)(T);
}

