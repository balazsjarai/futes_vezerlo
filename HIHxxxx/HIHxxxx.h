/*
 * HIHxxxx.h
 *
 * Created: 2020. 03. 23. 9:34:06
 *  Author: hubaja2
 */ 


#ifndef HIHXXXX_H_
#define HIHXXXX_H_

#include "../I2C/i2c_master.h"
#include <inttypes.h>

#define HIHxxxx_address	0x27 << 1


float HIHxxxxHumidity;
float HIHxxxxTemperature;
uint8_t HIHxxxxData[4];

uint8_t StartHIHxxxxMeasurement();
uint8_t ReadHIHxxxxMeasurement();

float ReadHIHxxxxHumidity();
float ReadHIHxxxxTemperature();



#endif /* HIHXXXX_H_ */



