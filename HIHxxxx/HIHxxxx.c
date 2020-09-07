/*
 * HIHxxxx.c
 *
 * Created: 2020. 03. 23. 9:34:20
 *  Author: hubaja2
 */ 

#include "HIHxxxx.h"


uint8_t StartHIHxxxxMeasurement()
{
	uint8_t state;
	state = i2c_start(HIHxxxx_address | I2C_WRITE);
	i2c_stop();
	return state;
}

uint8_t ReadHIHxxxxMeasurement()
{
	uint8_t state;
	state = i2c_receive(HIHxxxx_address, HIHxxxxData, 4);
	
	if (state == 0)
	{
		HIHxxxxHumidity = (HIHxxxxData[0] << 8);
		HIHxxxxHumidity += HIHxxxxData[1];
		
		HIHxxxxHumidity = (HIHxxxxHumidity / (0x4000 - 2)) * 100;
		
		HIHxxxxTemperature = HIHxxxxData[2] << 6;
		HIHxxxxTemperature += HIHxxxxData[3] >> 2;
		HIHxxxxTemperature = (HIHxxxxTemperature / (0x4000 - 2)) * 165 - 40;
	}
	return state;
}

float ReadHIHxxxxHumidity()
{	
	return HIHxxxxHumidity;
}

float ReadHIHxxxxTemperature()
{
	return HIHxxxxTemperature;
}
