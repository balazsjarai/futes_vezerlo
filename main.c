//TODO

// külsõ hõmérés, tiltás -> OK
// szivattyú utánfutás -> OK
// kazánház min / kivánt -> OK
// min < kivánt -> OK
// több kijelzés -> OK
// óra -> belsõ idõzítõvel -> OK
// külsõ hõmérõ hiba
// elõzõ / aktuális állapot -> kevesebb frissítés -> nem jó, mert beragadhat egy-egy relé
// dátum -> OK
// fûtés napközben, ha elegendõ a puffer energiája, de termosztát nem kér -> OK
// szabadság / távollét beállítás
// OpenTherm -> OK
// termosztát funkció
// osztó termoszelep vezérlés
// min/max hõmérséklet regisztrálás -> OK
// DS hõmérõ hiba -> OK
// UART érték lekérdezés/beírás -> teszt
// PID szabályzás tároló fûtésre
// túlmelegedés elleni védelem -> OK
// HMV kiegészítõ fûtés kapcsolás -> OK
// Ház szelep -> OK
// Puffer szivattyú letapadás elleni védelem -> OK
// Alapszintû külsõ hõmérséklet kompenzálás -> OK

#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/eeprom.h>

#include "main.h"
#include "MENU/menu.h"
#include "LCD/lcd.h"
#include "Utils/ftoa.h"
#include "UART/UART.h"
#include "SPI/spi.h"
#include "DS18B20/crc8.h"
#include "DS18B20/delay.h"
#include "DS18B20/ds18x20.h"
#include "DS18B20/onewire.h"
#include "I2C/i2c_master.h"
#include "HIHxxxx/HIHxxxx.h"

static const __flash uint8_t fraccel_lookup[16] =
{
	0x00U, 0x01U, 0x01U, 0x02U, 0x03U, 0x03U, 0x04U, 0x04U, 0x05U, 0x06U, 0x06U, 0x07U, 0x08U, 0x08U, 0x09U, 0x09U
};

volatile uint8_t TimerElapsed = 0;

char uartdata_in[15];
unsigned char uartdata_count;

uint8_t Mode = 0; uint8_t EEMEM eeMode = 0;
uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10;
uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight = 0;
uint16_t PumpPlusTime = 60; uint16_t EEMEM eePumpPlusTime = 60;
uint8_t ComfortMode = 0; uint8_t EEMEM eeComfortMode = 0;
uint16_t ComfortTemp = 2000; uint16_t EEMEM eeComfortTemp = 2000;
uint8_t ComfortForwardTemp = 35; uint8_t EEMEM eeComfortForwardTemp = 35;
uint16_t ComfortMaxTime = 1500; uint16_t EEMEM eeComfortMaxTime = 1500;
uint16_t ComfortMinTime = 1000; uint16_t EEMEM eeComfortMinTime = 1000;
uint16_t Restarts = 0; uint16_t EEMEM eeRestarts = 0;


float HIHxxxxTemp; char HIHxxxxTempBuf[10];
float HIHxxxxHumid; char HIHxxxxHumidBuf[6];
int16_t HIHxxxxTempInt;

TempSensor DHW, Buffer, Engineering, Garage, Floor, GasForward, GasReturn, Mixed, Forward, Return;

//uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
uint8_t DHWTempDesired, DHWTempMin;
uint16_t DHWMinTime, DHWMaxTime;

//char DHWTempActualBuf[4], DHWTempActualFracBuf[2];
uint8_t EEMEM eeDHWTempDesired = 50;
uint8_t EEMEM eeDHWTempMin = 40;
uint16_t EEMEM eeDHWMinTime = 1500;
uint16_t EEMEM eeDHWMaxTime = 2200;

//uint8_t BufferTempActual; char BufferTempActualBuf[4], BufferTempActualFracBuf[2];
uint8_t ForwardHeatTemp = 38; uint8_t EEMEM eeForwardHeatTemp = 38;
uint8_t BufferMaxTemp = 90; uint8_t EEMEM eeBufferMaxTemp = 90;
uint8_t GasForwardHeatTemp = 42; uint8_t EEMEM eeGasForwardHeatTemp = 42;

//uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin; char EngineeringTempActualBuf[4], EngineeringTempActualFracBuf[2];
uint8_t EngineeringTempDesired, EngineeringTempMin;
uint8_t EEMEM eeEngineeringTempDesired = 10;
uint8_t EEMEM eeEngineeringTempMin = 5;

//uint8_t GarageTemp;
//char GarageTempBuf[4], GarageTempFracBuf[2];
//
uint16_t LivingRoomTemp;
char LivingRoomTempBuf[4], LivingRoomTempFracBuf[2];
//
//uint8_t FloorTemp;
//char FloorTempBuf[4], FloorTempFracBuf[2];
//
//uint8_t ForwardTemp;
//char ForwardTempBuf[4], ForwardTempFracBuf[2];
//
//uint8_t ReturnTemp;
//char ReturnTempBuf[4], ReturnTempFracBuf[2];

float HIHxxxxTempMin, HIHxxxxTempMax;
//uint8_t DHWTempMinMeasured, DHWTempMax, BufferTempMin, BufferTempMax, EngineeringTempMinMeasured, EngineeringTempMax, GarageTempMin, GarageTempMax, FloorTempMin, FloorTempMax;
uint16_t LivingRoomTempMin, LivingRoomTempMax;

uint16_t SwitchOnOutdoorTempMin;
uint16_t EEMEM eeSwitchOnOutdoorTempMin = 2300;

unsigned char Relays = 0;

//uint8_t DHWSensorID = 1;
uint8_t EEMEM eeDHWSensorID = 1;

//uint8_t BufferSensorID = 5;
uint8_t EEMEM eeBufferSensorID = 5;

//uint8_t GarageSensorID = 6;
uint8_t EEMEM eeGarageSensorID = 6;

//uint8_t EngineeringSensorID = 0;
uint8_t EEMEM eeEngineeringSensorID = 0;

uint8_t LivingRoomSensorID = 3;
uint8_t EEMEM eeLivingRoomSensorID = 3;

//uint8_t FloorSensorID = 4;
uint8_t EEMEM eeFloorSensorID = 4;

//uint8_t ForwardTempSensorID = 2;
uint8_t EEMEM eeGasForwardTempSensorID = 2;

//uint8_t ReturnTempSensorID = 7;
uint8_t EEMEM eeGasReturnTempSensorID = 7;

//uint8_t ForwardTempSensorID = 2;
uint8_t EEMEM eeForwardTempSensorID = 9;

//uint8_t ReturnTempSensorID = 7;
uint8_t EEMEM eeReturnTempSensorID = 10;

//uint8_t ReturnTempSensorID = 7;
uint8_t EEMEM eeMixedTempSensorID = 8;

uint8_t nSensors;
uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];

uint8_t Hour = 0, Minute = 0, Seconds = 0;
uint8_t ClockInitialized = 0;

uint16_t Year = 2020;
uint8_t Month = 1, Day = 1, DayName = MONDAY;

uint8_t isInitialized = 0;
uint8_t PumpRunner = 0;

uint8_t GetFracCel(uint8_t cel_frac_bits)
{
	return fraccel_lookup[cel_frac_bits];
}

//ISR(ADC_vect) // LCD háttér világítás PWM
//{
	//if (ADC < 10)
		//OCR3C = 10;
	//else
		//OCR3C = ADC;
//}

int GetCompensation(int outside)
{
	int diff = 23 - outside;
	if (diff <= 10)
		return -10;
	else if (10 < diff && diff <= 20)
		return -5;
	else if (20 < diff && diff <= 30)
		return 0;
	else if (30 < diff && diff <= 40)
		return 5;
	else if (40 < diff )
		return 10;
	return 0;
	
}
ISR(TIMER1_COMPA_vect) // 1 mp-es idõzítõ
{
	if (TimerElapsed == 0);
		TimerElapsed++;
}

int TempToVariable(TempSensor * tempsensor, uint8_t * cel, uint8_t * celfracs)
{
	tempsensor->ActualTemp = *cel;
	utoa(tempsensor->ActualTemp, tempsensor->ActualTempBuffer, 10);
	utoa(*celfracs, tempsensor->ActualTempFracBuffer, 10);
	if (tempsensor->ActualTemp > tempsensor->MeasuredMaximumTemp)
		tempsensor->MeasuredMaximumTemp = tempsensor->ActualTemp;
	if (tempsensor->ActualTemp < tempsensor->MeasuredMinimumTemp)
		tempsensor->MeasuredMinimumTemp = tempsensor->ActualTemp;
	if (*cel > 100)
	{
		strcpy_P(tempsensor->ActualTempBuffer, PSTR("--"));
		strcpy_P(tempsensor->ActualTempFracBuffer, PSTR("-"));
		return 1;
	}
	return 0;
}

void SensorRead()
{
	//static uint8_t timer_state = BME280_temp_state;
	uint8_t i;
	uint8_t subzero, cel, cel_frac_bits;
	static uint8_t display = 20;
	static uint8_t timerstate = 0;
	uint8_t sensor_fail = 0;
	static uint8_t web = 0;
	uint8_t relmodlevel = 0;
	
	switch (timerstate)
	{
		case (HIHxxxxStartMeaserementState):
			StartHIHxxxxMeasurement();
			if (i2c_start(0x09 | I2C_READ) == 0)
			{		
				i2c_write('m');
				relmodlevel = i2c_read_ack();
				i2c_stop();
			}
			timerstate++;
		break;

		case (HIHxxxxReadMeasurementState):
			ReadHIHxxxxMeasurement();
			HIHxxxxTemp = HIHxxxxTemperature; // in °C
			memset(HIHxxxxTempBuf, 0x00, sizeof(HIHxxxxTempBuf));
			ftoa(HIHxxxxTempBuf, HIHxxxxTemp, 1);
			
			HIHxxxxTempInt = (int16_t)( HIHxxxxTemp * 100);
			if (Mode == DEBUG1)
				{ uart_puts_p(PSTR("S11;")); uart_puts(HIHxxxxTempBuf); uart_puts_p(PSTR("\r")); }
			if (HIHxxxxTemp > HIHxxxxTempMax)
				HIHxxxxTempMax = HIHxxxxTemp;
			if (HIHxxxxTemp < HIHxxxxTempMin)
				HIHxxxxTempMin = HIHxxxxTemp;
			
			HIHxxxxHumid = HIHxxxxHumidity;
			ftoa(HIHxxxxHumidBuf, HIHxxxxHumid, 1);
			if (Mode == DEBUG1)
				{ uart_puts_p(PSTR("S12;")); uart_puts(HIHxxxxHumidBuf); uart_puts_p(PSTR("\r")); }
			timerstate++;
		break;
		
		//case (HIHxxxxReadState):
			//ReadHIHxxxxMeasurement();
			//HIHxxxxTemp = HIHxxxxTemperature;
			//HIHxxxxHumid = HIHxxxxHumidity;
			//ftoa(HIHxxxxTempBuf, HIHxxxxTemp, 1);
			//ftoa(HIHxxxxHumidBuf, HIHxxxxHumid, 1);
			//timerstate++;
			//break;

		case (DS18B20State1):
			ow_set_bus(&DS18B20_PIN,&DS18B20_PORT,&DS18B20_DDR,DS18B20_PINx);
			DS18X20_start_meas(DS18X20_POWER_EXTERN,NULL);
			timerstate++;
		break;

		case (DS18B20State2):
			ow_set_bus(&DS18B20_PIN,&DS18B20_PORT,&DS18B20_DDR,DS18B20_PINx);
			i = 0;
			while (i < nSensors)
			{
				if ( DS18X20_read_meas( &gSensorIDs[i][0], &subzero, &cel, &cel_frac_bits) == DS18X20_OK )
				{
					if (Mode == DEBUG1)
						{ uart_puts_P("S"); uart_puti(i); uart_puts_P(";"); uart_puti(subzero); uart_puti(cel); uart_puts_P("."); uart_puti(GetFracCel(cel_frac_bits)); uart_puts_P("\r"); }
				}
				uint8_t celfracs = GetFracCel(cel_frac_bits);
				
				if (i == DHW.SensorID)
				{
					sensor_fail = TempToVariable(&DHW, &cel, &celfracs);
				}
				if (i == Buffer.SensorID)
				{
					sensor_fail = TempToVariable(&Buffer, &cel, &celfracs);
				}
				if (i == Engineering.SensorID)
				{
					sensor_fail = TempToVariable(&Engineering, &cel, &celfracs);
				}
				if (i == Garage.SensorID)
				{
					sensor_fail = TempToVariable(&Garage, &cel, &celfracs);
				}
				if (i == LivingRoomSensorID)
				{
					LivingRoomTemp = cel * 100;
					if (celfracs < 10)
						LivingRoomTemp += celfracs * 10;
					else
						LivingRoomTemp += celfracs;
					utoa(cel, LivingRoomTempBuf, 10);
					utoa(celfracs, LivingRoomTempFracBuf, 10);
					if (LivingRoomTemp > LivingRoomTempMax)
						LivingRoomTempMax = LivingRoomTemp;
					if (LivingRoomTemp < LivingRoomTempMin)
						LivingRoomTempMin = LivingRoomTemp;
					if (cel > 135)
					{
						strcpy_P(LivingRoomTempBuf, PSTR("--"));
						strcpy_P(LivingRoomTempFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == Floor.SensorID)
				{
					sensor_fail = TempToVariable(&Floor, &cel, &celfracs);
				}
				if (i == GasForward.SensorID)
				{
					sensor_fail = TempToVariable(&GasForward, &cel, &celfracs);
				}
				if (i == GasReturn.SensorID)
				{
					sensor_fail = TempToVariable(&GasReturn, &cel, &celfracs);
				}
				if (i == Forward.SensorID)
				{
					sensor_fail = TempToVariable(&Forward, &cel, &celfracs);
				}
				if (i == Return.SensorID)
				{
					sensor_fail = TempToVariable(&Return, &cel, &celfracs);
				}
				if (i == Mixed.SensorID)
				{
					sensor_fail = TempToVariable(&Mixed, &cel, &celfracs);
				}
				i++;
			}

			timerstate++;
		break;

		case (ReportState1):
			isInitialized = 1;
			if (web > 60)
			{
				uart_puts_p(PSTR("S3;")); uart_puts(DHW.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(DHW.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S4;")); uart_puts(Buffer.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Buffer.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S5;")); uart_puts(Engineering.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Engineering.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S6;")); uart_puts(Garage.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Garage.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S7;")); uart_puts(LivingRoomTempBuf); uart_puts_p(PSTR(".")); uart_puts(LivingRoomTempFracBuf); uart_puts_p(PSTR("\n"));
			}
			timerstate++;
		break;

		case (ReportState2):
			if (web > 60)
			{
				uart_puts_p(PSTR("S8;")); uart_puts(Floor.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Floor.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S9;")); uart_puts(GasForward.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(GasForward.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S10;")); uart_puts(GasReturn.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(GasReturn.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S11;")); uart_puts(HIHxxxxTempBuf);  uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S12;")); uart_puts(HIHxxxxHumidBuf); uart_puts_p(PSTR("\n"));
			}
			timerstate++;
		break;
			
		case (ReportState3):
			if (web > 60)
			{
				uart_puts_p(PSTR("S13;")); uart_puts(Forward.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Forward.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S14;")); uart_puts(Return.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Return.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S15;")); uart_puts(Mixed.ActualTempBuffer); uart_puts_p(PSTR(".")); uart_puts(Mixed.ActualTempFracBuffer); uart_puts_p(PSTR("\r"));
				uart_puts_p(PSTR("S16;")); uart_puti((int)relmodlevel); uart_puts_p(PSTR("\n"));
				web = 0;
			}
			timerstate++;
		break;

		default:
			timerstate = 0;
		break;
	}

	web++;

	if (sensor_fail != 0)
	{
		ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
		nSensors = search_sensors();
		sensor_fail = 0;
	}

	if (MenuTimer == 0)
	{
		
		if (display == 20)
		{
			lcd_clrscr();
			lcd_puts_p(PSTR("HMV ")); lcd_puts(DHW.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(DHW.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Puffer ")); lcd_puts(Buffer.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Buffer.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Gépház ")); lcd_puts(Engineering.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Engineering.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("Külsõ ")); lcd_puts(HIHxxxxTempBuf); lcd_puts_p(PSTR(" C"));
		}
		else if (display == 15)
		{
			lcd_clrscr();
			lcd_puts_hu(PSTR("Garázs ")); lcd_puts(Garage.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Garage.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Nappali ")); lcd_puts(LivingRoomTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(LivingRoomTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Padló ")); lcd_puts(Floor.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Floor.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
		}
		else if (display == 10)
		{
			lcd_clrscr();
			lcd_puts_hu(PSTR("Elõremenõ ")); lcd_puts(GasForward.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(GasForward.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("Visszatérõ ")); lcd_puts(GasReturn.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(GasReturn.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puti(Hour); lcd_puts_p(PSTR(":")); lcd_puti(Minute);
			lcd_gotoxy(0,3);
			lcd_puti(Year); lcd_puts_p(PSTR(".")); lcd_puti(Month); lcd_puts_p(PSTR(".")); lcd_puti(Day);
			lcd_gotoxy(12,3);
			switch (DayName)
			{
				case 1:
					lcd_puts_hu(PSTR("hétfõ"));
					break;
				case 2:
					lcd_puts_hu(PSTR("kedd"));
					break;
				case 3:
					lcd_puts_hu(PSTR("szerda"));
					break;
				case 4:
					lcd_puts_hu(PSTR("csüt"));
					break;
				case 5:
					lcd_puts_hu(PSTR("péntek"));
					break;
				case 6:
					lcd_puts_hu(PSTR("szombat"));
					break;
				case 7:
					lcd_puts_hu(PSTR("vasárnap"));
					break;
			}
		}
		else if (display == 5)
		{
			lcd_clrscr();
			lcd_puts_p(PSTR("HMV szelep ")); lcd_putbit(Relays, DHW_VALVE_RELAY); lcd_gotoxy(15,0);
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("Gáz relé ")); lcd_putbit(Relays, GAS_RELAY);
			lcd_gotoxy(0,2);
			lcd_puts_p(PSTR("Puffer sz/p ")); lcd_putbit(Relays, BUFFER_VALVE_RELAY); lcd_putbit(Relays, BUFFER_PUMP_RELAY);
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("Föld/Emel ")); lcd_putbit(Relays, FIRST_FLOOR_VALVE); lcd_putbit(Relays, SECOND_FLOOR_VALVE);
		}
		if (--display == 0)
			display = 20;
		//display--;
		//lcd_puts_hu(PSTR("Relek: ")); lcd_putbyte_bin(Relays);
	}
	else
		MenuTimer--;

	//if (LCDBackLight == 0)
		//ADCSRA |= (1<<ADSC);

	return;
}

void CheckConditions()
{
	// DHW felsõ kör
	static uint16_t pumpplustime = 0;
	uint16_t currTime = Hour * 100 + Minute;
	uint8_t DHW_condition;
	uint8_t oldRelays = Relays;
	uint8_t activeCondition = 0;
	if (isInitialized)
	{
		if (ClockInitialized)
		{
			DHW_condition = (DHW.ActualTemp < DHWTempDesired && (DHWMaxTime >= currTime && DHWMinTime <= currTime )) ? 1 : 0;
			if (DHWMaxTime >= currTime && DHWMinTime <= currTime )
				Relays |= (1 << DHW_AUX_RELAY);
			else
				Relays &= ~(1 << DHW_AUX_RELAY);
		}
		else
			DHW_condition = (DHW.ActualTemp < DHWTempDesired) ? 1 : 0;

		if (DHW_condition) // alacsony HMV hõmérséklet
		{
			if (DHWTempMin >= DHW.ActualTemp)
			{
				activeCondition = 1;
				Relays |= (1 << DHW_VALVE_RELAY);
				Relays &= ~((1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));				

				if (Buffer.ActualTemp < DHWTempDesired) // Pufferben nincs elég energia
				{
					pumpplustime = PumpPlusTime;
					Relays |= (1 << GAS_RELAY);
					Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
					if (i2c_start(0x09 | I2C_WRITE) == 0)
					{				
						i2c_write('c');
						i2c_write(DHWTempDesired + 15);
						i2c_stop();
					}
					//if (DebugMode > 0)
					//	uart_puts_P("DHW relay activated with gas\n");
				}
				else // Pufferben van elég energia
				{
					if (!pumpplustime)
					{
						pumpplustime--;
						Relays &= ~(1 << GAS_RELAY);
						if (i2c_start(0x09 | I2C_WRITE) == 0)
						{
							i2c_write('o');
							i2c_write(0);
							i2c_stop();
						}
					}
					else
					{
						Relays |= (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY);
						
					}
					//if (DebugMode > 0)
					//	uart_puts_P("DHW relay activated with buffer\n");
				}

			}
		}
		else // megszûnõ hõigény esetén az utókeringetés a gépház radiátorát fûti, különben visszahûtené a HMV-t
		{
			if (((!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) 
				|| (!(THERMOSTAT_PIN & (1 << FIRST_REMOTE_PIN)) || !(PIND & (1 << SECOND_REMOTE_PIN)))
				|| (Engineering.ActualTemp < EngineeringTempDesired)))
				&& (HIHxxxxTempInt <= SwitchOnOutdoorTempMin))
			{
				activeCondition = 1;
				if (Engineering.ActualTemp < EngineeringTempMin	|| !(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << FIRST_REMOTE_PIN)) || !(PIND & (1 << SECOND_REMOTE_PIN)))
				{
					if ((Buffer.ActualTemp >= ForwardHeatTemp) || (Buffer.ActualTemp <= ForwardHeatTemp && Forward.ActualTemp >= ForwardHeatTemp && !pumpplustime))
					{
						if (!pumpplustime) // ha átkapcsol gázról pufferre, a gáz még utókeringetne
						{							
							Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));							
						}
						else
						{
							pumpplustime--;
							Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
							
							if (i2c_start(0x09 | I2C_WRITE) == 0)
							{
								i2c_write('o');
								i2c_write(0);
								i2c_stop();
							}
						}
					}
					else
					{
						pumpplustime = PumpPlusTime; // gáz utókeringetés
						Relays |= (1 << GAS_RELAY);
						Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
						int comp = GetCompensation((int16_t)( HIHxxxxTemp));
						if (i2c_start(0x09 | I2C_WRITE) == 0)
						{							
							i2c_write('c');
							i2c_write(GasForwardHeatTemp + comp);
							i2c_stop();
						}
					}								
				}

				if (!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || (!(THERMOSTAT_PIN & (1 << FIRST_REMOTE_PIN))))
				{
					Relays |= (1 << FIRST_FLOOR_VALVE);
					uart_puts("F1\r\n");
				}
				else
				{
					Relays &= ~(1 << FIRST_FLOOR_VALVE);
					uart_puts("F0\r\n");
				}

				if (!(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) || !(PIND & (1 << SECOND_REMOTE_PIN)))
				{
					Relays |= (1 << SECOND_FLOOR_VALVE);
					uart_puts("E1\r\n");
				}
				else
				{
					Relays &= ~(1 << SECOND_FLOOR_VALVE);
					uart_puts("E0\r\n");
				}
				if (!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << FIRST_REMOTE_PIN)) || !(PIND & (1 << SECOND_REMOTE_PIN)))
				{
					Relays |= 1 << (HOUSE_VALVE_RELAY);
				}
				else
				{
					Relays &= ~(1 << HOUSE_VALVE_RELAY);
				}
			}
			else
			{
				if (!pumpplustime)
					Relays &= ~((1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));
				else
					pumpplustime--;

				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 <<BUFFER_PUMP_RELAY) | (1 << GAS_RELAY));
				
				if (i2c_start(0x09 | I2C_WRITE) == 0)
				{
					i2c_write('o');
					i2c_write(0);
					i2c_stop();
				}
				//if (DebugMode > 0)
				//	uart_puts_P("GAS and FIRST_FLOOR_VALVE, SECOND_FLOOR_VALVE relay deactivated\n");

				if (ClockInitialized && ComfortMode && !activeCondition)
				{
					if (ComfortMaxTime >= currTime && ComfortMinTime <= currTime && LivingRoomTemp <= ComfortTemp && Buffer.ActualTemp >= ComfortForwardTemp)
					{
						if (!pumpplustime) // ha átkapcsol gázról pufferre, a gáz még utókeringetne
						{
							Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << FIRST_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));
						}
						else
						{
							pumpplustime--;
							Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
						
							if (i2c_start(0x09 | I2C_WRITE) == 0)
							{
								i2c_write('o');
								i2c_write(0);
								i2c_stop();
							}
						}
					}
					else
						Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << FIRST_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));
				}
			}
		}

		if (Buffer.ActualTemp >= (BufferMaxTemp - 5))
		{
			if (Buffer.ActualTemp >= BufferMaxTemp)
				Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
			else if (Buffer.ActualTemp <= (BufferMaxTemp - 5))
				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
		}
		
		if (!activeCondition && !pumpplustime && (PumpRunner > 0))
		{
			PumpRunner--;
			if (PumpRunner == 0)
			{
				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));
			}
			else
			{
				Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE) | (1 << HOUSE_VALVE_RELAY));
			}
		}

		if (Relays != oldRelays)
		{
			uart_puts_P("S99;");uart_puthex_byte(Relays);uart_puts_P("\r\n");
		}
		SwitchRelays();
	}
}

uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;

	/* clear display and home cursor */
	if (Mode == DEBUG1)
		uart_puts_P("Bus scanning ...\n");

	nSensors = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && nSensors < DS18B20_MAX_NO;)
	{
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			if (Mode == DEBUG1)
				uart_puts_P("No sensor found\n");
			break;
		}

		if( diff == OW_DATA_ERR ) {
			if (Mode == DEBUG1)
				uart_puts_P("Bus error\n");
			break;
		}

		for (i=0;i<OW_ROMCODE_SIZE;i++)
		{
			gSensorIDs[nSensors][i]=id[i];
		}

		nSensors++;
	}

	return nSensors;
}

void SwitchRelays()
{
	SPIInit();
	SPIWrite(RELAYS_CS_PIN, Relays);
	SPIClose();
}

void read_from_eeprom()
{
	DHW.SensorID = eeprom_read_byte(&eeDHWSensorID);
	DHWTempDesired = eeprom_read_byte(&eeDHWTempDesired);
	DHWTempMin = eeprom_read_byte(&eeDHWTempMin);
	DHWMinTime = eeprom_read_word(&eeDHWMinTime);
	DHWMaxTime = eeprom_read_word(&eeDHWMaxTime);
	Buffer.SensorID = eeprom_read_byte(&eeBufferSensorID);
	ForwardHeatTemp = eeprom_read_byte(&eeForwardHeatTemp);
	BufferMaxTemp = eeprom_read_byte(&eeBufferMaxTemp);
	Engineering.SensorID = eeprom_read_byte(&eeEngineeringSensorID);
	EngineeringTempDesired = eeprom_read_byte(&eeEngineeringTempDesired);
	EngineeringTempMin = eeprom_read_byte(&eeEngineeringTempMin);
	Garage.SensorID = eeprom_read_byte(&eeGarageSensorID);
	LivingRoomSensorID = eeprom_read_byte(&eeLivingRoomSensorID);
	Floor.SensorID = eeprom_read_byte(&eeFloorSensorID);
	GasForward.SensorID = eeprom_read_byte(&eeGasForwardTempSensorID);
	GasReturn.SensorID = eeprom_read_byte(&eeGasReturnTempSensorID);
	Forward.SensorID = eeprom_read_byte(&eeForwardTempSensorID);
	Return.SensorID = eeprom_read_byte(&eeReturnTempSensorID);
	Mixed.SensorID = eeprom_read_byte(&eeMixedTempSensorID);
	SwitchOnOutdoorTempMin = eeprom_read_word(&eeSwitchOnOutdoorTempMin);
	Mode = eeprom_read_byte(&eeMode);
	MenuTimer = eeprom_read_byte(&eeMenuTimer);
	LCDBackLight = eeprom_read_byte(&eeLCDBackLight);
	PumpPlusTime = eeprom_read_word(&eePumpPlusTime);
	ComfortMode = eeprom_read_byte(&eeComfortMode);
	ComfortTemp = eeprom_read_word(&eeComfortTemp);
	ComfortForwardTemp = eeprom_read_byte(&eeComfortForwardTemp);
	ComfortMaxTime = eeprom_read_word(&eeComfortMaxTime);
	ComfortMinTime = eeprom_read_word(&eeComfortMinTime);
	eeGasForwardHeatTemp = eeprom_read_byte(&eeGasForwardHeatTemp);
	Restarts = eeprom_read_word(&eeRestarts);
}

int main(void)
{
	char readchar;
	read_from_eeprom();
	eeprom_update_word(&eeRestarts, ++Restarts);

	lcd_init(LCD_DISP_ON);
	lcd_defc(magyar_betuk);

	TCCR3A |= (1 << WGM30)|(1<<COM3C1);
	TCCR3B |= (1 << WGM32)|(1 << CS30)|(1 << CS31);
	OCR3C = 60;
	DDRE |= (1 << PINE5);	//LCD led PWM
	PORTE |= (1 << PINE5);	//LCD led PWM

	lcd_clrscr();
	lcd_puts_hu(PSTR("Fûtés vezérlés v0.93"));

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);

	BUZZER_DDR |= (1 << BUZZER_PIN);

	THERMOSTAT_DDR &= ~((1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN)|(1 << FIRST_REMOTE_PIN));
	DDRD &= ~(1 << SECOND_REMOTE_PIN);
	//THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);

	_delay_ms(10);
	SPIInit();
	SPIWrite(3, 0x00);
	SPIClose();

	Relays = 0x00;
	SwitchRelays();

	lcd_gotoxy(0,1);
	lcd_puts_hu(PSTR("Alapállapotba állás"));
	lcd_gotoxy(0,2);
	uint8_t init_counter = 0;
	while (init_counter < 10)
	{
		lcd_putc('x');
		_delay_ms(1000);
		init_counter++;
	}

	lcd_clrscr();
	lcd_puts_hu(PSTR("I2C indítás"));
	i2c_init();
	lcd_gotoxy(0,1);
	//lcd_puts_hu(PSTR("HIHxxxx indítás"));
	lcd_puts_hu(PSTR("Megszakítások eng."));
	
	//init_BME280();

	sei();
	uart_puts_p(PSTR("Interrupt enabled\n"));


	ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
	nSensors = search_sensors();

	if (Mode == DEBUG1)
		{ uart_puts_P("Found "); uart_puti(nSensors); uart_puts_P(" DS18B20 sensors\n"); }
	lcd_gotoxy(0,2);
	lcd_puts_p(PSTR("DS18B20: ")); lcd_puti(nSensors); lcd_puts_p(PSTR(" db"));

	_delay_ms(3000);

	TCCR1B |= (1 << CS12) | (1 << WGM12);
	TCNT1 = 0;
	OCR1A = 62500;//14400; // 1000ms
	TIMSK |= (1 << OCIE1A);

	//ADMUX |= (1<<REFS0) | (1<< REFS1);
	//ADCSRA |= (1 << ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);

	//reset watchdog
	wdt_reset();
	WDTCR |= (1<<WDE) | (1 << WDCE);
	WDTCR |= (1<<WDP2)| (1<<WDP1) | (1<<WDP0);


	menuInit();

	while(1)
	{
		if (TimerElapsed == 1)
		{
			if (++Seconds == 60)
			{
				Seconds = 0;
				if (++Minute == 60)
				{
					Minute = 0;
					if (++Hour == 24)
					{
						PumpRunner = 60;
						Hour = 0;

						if (++DayName == 8)
							DayName = 1;
						uint8_t xDay;
						switch (Month)
						{
							case 1:
							case 3:
							case 5:
							case 7:
							case 8:
							case 10:
							case 12:
								xDay = 32;
								break;

							case 4:
							case 6:
							case 9:
							case 11:
								xDay = 31;
								break;

							case 2:
								xDay = 29;
								break;
						}
						if (++Day == xDay)
						{
							Day = 1;
							if (++Month == 13)
							{
								Month = 1;
								Year++;
							}
						}
					}

				}
			}

			SensorRead();
			if (Mode != MANUAL)
				CheckConditions();
			TimerElapsed = 0;
		}

		wdt_reset();
		menuPollButtons();

		if (Mode == DEBUG1)
		{
			readchar = uart_getc();
			if (readchar != UART_NO_DATA)
			{
				if (readchar != CHAR_NEWLINE)
				{
					uartdata_in[uartdata_count++] = readchar;
				}
				else
				{
					uartdata_in[uartdata_count] = CHAR_NEWLINE;

					char *pch;
					char *pch2;
					char cmdvalue[10];
					char cmdvalue2[5];
					pch = strchr(uartdata_in, ';');
					pch2 = strchr(cmdvalue, ',');

					if (pch2 == NULL)
						strcpy(cmdvalue, pch + 1);
					else
					{
						strlcpy(cmdvalue, pch, strlen(pch2));
						strcpy(cmdvalue2, pch2 + 1);
					}

					if (uartdata_in[0] == 'R') // read from EEPROM
					{
						if (uartdata_in[1] == '1')
						{
							uart_putc(eeprom_read_byte((uint8_t*) atoi(cmdvalue)));
						}
						else if (uartdata_in[1] == '2')
						{
							char readvalue[6];
							itoa(eeprom_read_word((uint16_t*) atoi(cmdvalue)), readvalue, 10);
							uart_puts(readvalue);
						}
					}
					else if (uartdata_in[0] == 'S') //read sensor
					{
						uint8_t sensorID;
						sensorID = atoi(pch);
						switch (sensorID)
						{
							case '1':
								uart_puts(DHW.ActualTempBuffer); uart_puts_P("."); uart_puts(DHW.ActualTempFracBuffer);
								break;
							case '2':
								uart_puts(Buffer.ActualTempBuffer); uart_puts_P("."); uart_puts(Buffer.ActualTempFracBuffer);
								break;
							case '3':
								uart_puts(Engineering.ActualTempBuffer); uart_puts_P("."); uart_puts(Engineering.ActualTempFracBuffer);
								break;
							case '4':
								uart_puts(Garage.ActualTempBuffer); uart_puts_P("."); uart_puts(Garage.ActualTempFracBuffer);
								break;
							case '5':
								uart_puts(LivingRoomTempBuf); uart_puts_P("."); uart_puts(LivingRoomTempFracBuf);
								break;
							case '6':
								uart_puts(GasForward.ActualTempBuffer); uart_puts_P("."); uart_puts(GasForward.ActualTempFracBuffer);
								break;
							case '7':
								uart_puts(GasReturn.ActualTempBuffer); uart_puts_P("."); uart_puts(GasReturn.ActualTempFracBuffer);
								break;
							case '8':
								uart_puts(HIHxxxxTempBuf);
								break;
						}
					}
					else if (uartdata_in[0] == 'W') // write to EEPROM
					{
						if (uartdata_in[1] == '1')
						{
							eeprom_update_byte((uint8_t*) atoi(cmdvalue), atoi(cmdvalue2));
						}
						else if (uartdata_in[1] == '2')
						{
							eeprom_update_word((uint16_t*) atoi(cmdvalue), atoi(cmdvalue2));
						}
					}
					else if (uartdata_in[0] == 'U') // update
					{
						read_from_eeprom();
					}
					else if (uartdata_in[0] == 'M')
					{
						Relays = atoi(cmdvalue);
						SwitchRelays();
					}
					
					uartdata_count = 0;
				}
			}
		}

	}
	uart_puts_p(PSTR("Fatal error, program end\r"));
}