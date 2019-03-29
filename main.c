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
// OpenTherm
// termosztát funkció
// osztó termoszelep vezérlés
// min/max hõmérséklet regisztrálás -> OK
// DS hõmérõ hiba -> OK
// UART érték lekérdezés/beírás -> teszt
// PID szabályzás tároló fûtésre
// túlmelegedés elleni védelem -> teszt

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
#include "BME280/bme280.h"
#include "Utils/ftoa.h"
#include "UART/UART.h"
#include "SPI/spi.h"
#include "DS18B20/crc8.h"
#include "DS18B20/delay.h"
#include "DS18B20/ds18x20.h"
#include "DS18B20/onewire.h"

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

float BME280Temp; char BME280TempBuf[8];
float BME280Humid; char BME280HumidBuf[6];
int16_t BME280TempInt;

uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
uint16_t DHWMinTime, DHWMaxTime;

char DHWTempActualBuf[4], DHWTempActualFracBuf[2];
uint8_t EEMEM eeDHWTempDesired = 50;
uint8_t EEMEM eeDHWTempMin = 40;
uint16_t EEMEM eeDHWMinTime = 1500;
uint16_t EEMEM eeDHWMaxTime = 2200;

uint8_t BufferTempActual; char BufferTempActualBuf[4], BufferTempActualFracBuf[2];
uint8_t ForwardHeatTemp = 38; uint8_t EEMEM eeForwardHeatTemp = 38;
uint8_t BufferMaxTemp = 90; uint8_t EEMEM eeBufferMaxTemp = 90;

uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin; char EngineeringTempActualBuf[4], EngineeringTempActualFracBuf[2];
uint8_t EEMEM eeEngineeringTempDesired = 10;
uint8_t EEMEM eeEngineeringTempMin = 5;

uint8_t GarageTemp;
char GarageTempBuf[4], GarageTempFracBuf[2];

uint16_t LivingRoomTemp;
char LivingRoomTempBuf[4], LivingRoomTempFracBuf[2];

uint8_t FloorTemp;
char FloorTempBuf[4], FloorTempFracBuf[2];

uint8_t ForwardTemp;
char ForwardTempBuf[4], ForwardTempFracBuf[2];

uint8_t ReturnTemp;
char ReturnTempBuf[4], ReturnTempFracBuf[2];

float BME280TempMin, BME280TempMax;
uint8_t DHWTempMinMeasured, DHWTempMax, BufferTempMin, BufferTempMax, EngineeringTempMinMeasured, EngineeringTempMax, GarageTempMin, GarageTempMax, FloorTempMin, FloorTempMax;
uint16_t LivingRoomTempMin, LivingRoomTempMax;

uint16_t SwitchOnOutdoorTempMin;
uint16_t EEMEM eeSwitchOnOutdoorTempMin = 2300;

unsigned char Relays = 0;

uint8_t DHWSensorID = 1;
uint8_t EEMEM eeDHWSensorID = 1;

uint8_t BufferSensorID = 5;
uint8_t EEMEM eeBufferSensorID = 5;

uint8_t GarageSensorID = 6;
uint8_t EEMEM eeGarageSensorID = 6;

uint8_t EngineeringSensorID = 0;
uint8_t EEMEM eeEngineeringSensorID = 0;

uint8_t LivingRoomSensorID = 3;
uint8_t EEMEM eeLivingRoomSensorID = 3;

uint8_t FloorSensorID = 4;
uint8_t EEMEM eeFloorSensorID = 4;

uint8_t ForwardTempSensorID = 2;
uint8_t EEMEM eeForwardTempSensorID = 2;

uint8_t ReturnTempSensorID = 7;
uint8_t EEMEM eeReturnTempSensorID = 7;

uint8_t nSensors;
uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];

uint8_t Hour = 0, Minute = 0, Seconds = 0;
uint8_t ClockInitialized = 0;

uint16_t Year = 2019;
uint8_t Month = 1, Day = 1, DayName = MONDAY;

uint8_t isInitialized = 0;

uint8_t GetFracCel(uint8_t cel_frac_bits)
{
	return fraccel_lookup[cel_frac_bits];
}

ISR(ADC_vect) // LCD háttér világítás PWM
{
	if (ADC < 10)
		OCR3C = 10;
	else
		OCR3C = ADC;
}

ISR(TIMER1_COMPA_vect) // 1 mp-es idõzítõ
{
	if (TimerElapsed == 0);
		TimerElapsed++;
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

	switch (timerstate)
	{
		case (BME280TempState):
			BME280Temp = bme280_readTemperature();
			ftoa(BME280TempBuf, BME280Temp, 1);
			//dtostrf(BME280Temp, 6, 1, BME280TempBuf);
			//sprintf(BME280TempBuf, "%f", BME280Temp);
			BME280TempInt = (int16_t)( BME280Temp * 100);
			if (Mode == DEBUG1)
				{ uart_puts_p(PSTR("S11;")); uart_puts(BME280TempBuf); uart_puts_p(PSTR("\r")); }
			timerstate++;
			if (BME280Temp > BME280TempMax)
				BME280TempMax = BME280Temp;
			if (BME280Temp < BME280TempMin)
				BME280TempMin = BME280Temp;
		break;

		case (BME280HumidState):
			BME280Humid = bme280_readHumidity();
			ftoa(BME280HumidBuf, BME280Humid, 1);
			if (Mode == DEBUG1)
				{ uart_puts_p(PSTR("S12;")); uart_puts(BME280HumidBuf); uart_puts_p(PSTR("\r")); }
			timerstate++;
		break;

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
				if (i == DHWSensorID)
				{
					DHWTempActual = cel;
					utoa(DHWTempActual, DHWTempActualBuf, 10);
					utoa(celfracs, DHWTempActualFracBuf, 10);
					if (DHWTempActual > DHWTempMax)
						DHWTempMax = DHWTempActual;
					if (DHWTempActual < DHWTempMinMeasured)
						DHWTempMinMeasured = DHWTempActual;
					if (cel > 135)
					{
						strcpy_P(DHWTempActualBuf, PSTR("--"));
						strcpy_P(DHWTempActualFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == BufferSensorID)
				{
					BufferTempActual = cel;
					utoa(BufferTempActual, BufferTempActualBuf, 10);
					utoa(celfracs, BufferTempActualFracBuf, 10);
					if (BufferTempActual > BufferTempMax)
						BufferTempMax = BufferTempActual;
					if (BufferTempActual < BufferTempMin)
						BufferTempMin = BufferTempActual;
					if (cel > 135)
					{
						strcpy_P(BufferTempActualBuf, PSTR("--"));
						strcpy_P(BufferTempActualFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == EngineeringSensorID)
				{
					EngineeringTempActual = cel;
					utoa(EngineeringTempActual, EngineeringTempActualBuf, 10);
					utoa(celfracs, EngineeringTempActualFracBuf, 10);
					if (EngineeringTempActual > EngineeringTempMax)
						EngineeringTempMax = EngineeringTempActual;
					if (EngineeringTempActual < EngineeringTempMinMeasured)
						EngineeringTempMinMeasured = EngineeringTempActual;
					if (cel > 135)
					{
						strcpy_P(EngineeringTempActualBuf, PSTR("--"));
						strcpy_P(EngineeringTempActualFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == GarageSensorID)
				{
					GarageTemp = cel;
					utoa(GarageTemp, GarageTempBuf, 10);
					utoa(celfracs, GarageTempFracBuf, 10);
					if (GarageTemp > GarageTempMax)
						GarageTempMax = GarageTemp;
					if (GarageTemp < GarageTempMin)
						GarageTempMin = GarageTemp;
					if (cel > 135)
					{
						strcpy_P(GarageTempBuf, PSTR("--"));
						strcpy_P(GarageTempFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
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
				if (i == FloorSensorID)
				{
					FloorTemp = cel;
					utoa(FloorTemp, FloorTempBuf, 10);
					utoa(celfracs, FloorTempFracBuf, 10);
					if (FloorTemp > FloorTempMax)
						FloorTempMax = FloorTemp;
					if (FloorTemp < FloorTempMin)
						FloorTempMin = FloorTemp;
					if (cel > 135)
					{
						strcpy_P(FloorTempBuf, PSTR("--"));
						strcpy_P(FloorTempFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == ForwardTempSensorID)
				{
					ForwardTemp = cel;
					utoa(ForwardTemp, ForwardTempBuf, 10);
					utoa(celfracs, ForwardTempFracBuf, 10);
					if (cel > 135)
					{
						strcpy_P(ForwardTempBuf, PSTR("--"));
						strcpy_P(ForwardTempFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				if (i == ReturnTempSensorID)
				{
					ReturnTemp = cel;
					utoa(ReturnTemp, ReturnTempBuf, 10);
					utoa(celfracs, ReturnTempFracBuf, 10);
					if (cel > 135)
					{
						strcpy_P(ReturnTempBuf, PSTR("--"));
						strcpy_P(ReturnTempFracBuf, PSTR("-"));
						sensor_fail = 1;
					}
				}
				i++;
			}

			timerstate++;
		break;

		case (ReportState1):
			isInitialized = 1;
			if (web > 120)
			{
				uart_puts_p(PSTR("S3;")); uart_puts(DHWTempActualBuf); uart_puts_p(PSTR(".")); uart_puts(DHWTempActualFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S4;")); uart_puts(BufferTempActualBuf); uart_puts_p(PSTR(".")); uart_puts(BufferTempActualFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S5;")); uart_puts(EngineeringTempActualBuf); uart_puts_p(PSTR(".")); uart_puts(EngineeringTempActualFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S6;")); uart_puts(GarageTempBuf); uart_puts_p(PSTR(".")); uart_puts(GarageTempFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S7;")); uart_puts(LivingRoomTempBuf); uart_puts_p(PSTR(".")); uart_puts(LivingRoomTempFracBuf); uart_puts_p(PSTR("\r"));
			}
			timerstate++;
		break;

		case (ReportState2):
			if (web > 120)
			{
				uart_puts_p(PSTR("S8;")); uart_puts(FloorTempBuf); uart_puts_p(PSTR(".")); uart_puts(FloorTempFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S9;")); uart_puts(ForwardTempBuf); uart_puts_p(PSTR(".")); uart_puts(ForwardTempFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S10;")); uart_puts(ReturnTempBuf); uart_puts_p(PSTR(".")); uart_puts(ReturnTempFracBuf); uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S11;")); uart_puts(BME280TempBuf);  uart_puts_p(PSTR("\n"));
				uart_puts_p(PSTR("S12;")); uart_puts(BME280HumidBuf); uart_puts_p(PSTR("\r"));
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
			lcd_puts_p(PSTR("HMV ")); lcd_puts(DHWTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(DHWTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Puffer ")); lcd_puts(BufferTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(BufferTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Gépház ")); lcd_puts(EngineeringTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(EngineeringTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("Külsõ ")); lcd_puts(BME280TempBuf); lcd_puts_p(PSTR(" C"));
		}
		else if (display == 15)
		{
			lcd_clrscr();
			lcd_puts_hu(PSTR("Garázs ")); lcd_puts(GarageTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(GarageTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Nappali ")); lcd_puts(LivingRoomTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(LivingRoomTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Padló ")); lcd_puts(FloorTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(FloorTempFracBuf); lcd_puts_p(PSTR(" C"));
		}
		else if (display == 10)
		{
			lcd_clrscr();
			lcd_puts_hu(PSTR("Elõremenõ ")); lcd_puts(ForwardTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(ForwardTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("Visszatérõ ")); lcd_puts(ReturnTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(ReturnTempFracBuf); lcd_puts_p(PSTR(" C"));
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

	if (LCDBackLight == 0)
		ADCSRA |= (1<<ADSC);

	return;
}

void CheckConditions()
{
	// DHW felsõ kör
	static uint16_t pumpplustime = 0;
	uint16_t currTime = Hour * 100 + Minute;
	uint8_t DHW_condition;
	uint8_t oldRelays = Relays;
	if (isInitialized)
	{
		if (ClockInitialized)
			DHW_condition = (DHWTempActual < DHWTempDesired && (DHWMaxTime >= currTime && DHWMinTime <= currTime )) ? 1 : 0;
		else
			DHW_condition = (DHWTempActual < DHWTempDesired) ? 1 : 0;

		if (DHW_condition) // alacsony HMV hõmérséklet
		{
			if (DHWTempMin >= DHWTempActual)
			{
				Relays |= (1 << DHW_VALVE_RELAY);
				Relays &= ~((1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE));

				if (BufferTempActual < DHWTempDesired) // Pufferben nincs elég energia
				{
					pumpplustime = PumpPlusTime;
					Relays |= (1 << GAS_RELAY);
					Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
					//if (DebugMode > 0)
					//	uart_puts_P("DHW relay activated with gas\n");
				}
				else // Pufferben van elég energia
				{
					if (pumpplustime > 0)
						pumpplustime--;
					else
					{
						Relays |= (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY);
						Relays &= ~(1 << GAS_RELAY);
					}
					//if (DebugMode > 0)
					//	uart_puts_P("DHW relay activated with buffer\n");
				}

			}
		}
		else // megszûnõ hõigény esetén az utókeringetés a gépház radiátorát fûti, különben visszahûtené a HMV-t
		{
			if ((!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN))
				|| (EngineeringTempActual < EngineeringTempDesired))
				&& (BME280TempInt <= SwitchOnOutdoorTempMin))
			{
				if (EngineeringTempActual < EngineeringTempMin
					|| !(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
				{
					if (BufferTempActual < ForwardHeatTemp) // Pufferben nincs elég energia
					{
						pumpplustime = PumpPlusTime; // gáz utókeringetés
						Relays |= (1 << GAS_RELAY);
						Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
					}
					else // Puffer elég meleg
					{
						if (!pumpplustime) // ha átkapcsol gázról pufferre, a gáz még utókeringetne
						{
							Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
							Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
						}
						else
							pumpplustime--;
					}
				}

				if (!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)))
					Relays |= (1 << FIRST_FLOOR_VALVE);
				else
					Relays &= ~(1 << FIRST_FLOOR_VALVE);

				if (!(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
					Relays |= (1 << SECOND_FLOOR_VALVE);
				else
					Relays &= ~(1 << SECOND_FLOOR_VALVE);
			}
			else
			{
				if (!pumpplustime)
					Relays &= ~((1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE));
				else
					pumpplustime--;

				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 <<BUFFER_PUMP_RELAY) | (1 << GAS_RELAY));
				//if (DebugMode > 0)
				//	uart_puts_P("GAS and FIRST_FLOOR_VALVE, SECOND_FLOOR_VALVE relay deactivated\n");

				if (ClockInitialized && ComfortMode)
				{
					if (ComfortMaxTime >= currTime && ComfortMinTime <= currTime)
					{
						if ((LivingRoomTemp <= ComfortTemp) && (BufferTempActual >= ComfortForwardTemp))
						{
							if (!pumpplustime) // ha átkapcsol gázról pufferre, a gáz még utókeringetne
							{
								Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
								Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << FIRST_FLOOR_VALVE));
							}
						}
						else
							Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
					}
					else
							Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
				}
			}


		}

		if (BufferTempActual >= (BufferMaxTemp - 5))
		{
			if (BufferTempActual >= BufferMaxTemp)
				Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
			else if (BufferTempActual <= (BufferMaxTemp - 5))
				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
		}

		if (Relays != oldRelays)
		{
			uart_puts_P("S99;");uart_puthex_byte(Relays);uart_puts_P("\r");
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
	if (Mode == DEBUG)
		uart_puts_P("Bus scanning ...\n");

	nSensors = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && nSensors < DS18B20_MAX_NO;)
	{
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			if (Mode == DEBUG)
				uart_puts_P("No sensor found\n");
			break;
		}

		if( diff == OW_DATA_ERR ) {
			if (Mode == DEBUG)
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
	DHWSensorID = eeprom_read_byte(&eeDHWSensorID);
	DHWTempDesired = eeprom_read_byte(&eeDHWTempDesired);
	DHWTempMin = eeprom_read_byte(&eeDHWTempMin);
	DHWMinTime = eeprom_read_word(&eeDHWMinTime);
	DHWMaxTime = eeprom_read_word(&eeDHWMaxTime);
	BufferSensorID = eeprom_read_byte(&eeBufferSensorID);
	ForwardHeatTemp = eeprom_read_byte(&eeForwardHeatTemp);
	BufferMaxTemp = eeprom_read_byte(&eeBufferMaxTemp);
	EngineeringSensorID = eeprom_read_byte(&eeEngineeringSensorID);
	EngineeringTempDesired = eeprom_read_byte(&eeEngineeringTempDesired);
	EngineeringTempMin = eeprom_read_byte(&eeEngineeringTempMin);
	GarageSensorID = eeprom_read_byte(&eeGarageSensorID);
	LivingRoomSensorID = eeprom_read_byte(&eeLivingRoomSensorID);
	FloorSensorID = eeprom_read_byte(&eeFloorSensorID);
	ForwardTempSensorID = eeprom_read_byte(&eeForwardTempSensorID);
	ReturnTempSensorID = eeprom_read_byte(&eeReturnTempSensorID);
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
	OCR3C = 128;
	DDRE |= (1 << PINE5);	//LCD led PWM
	PORTE |= (1 << PINE5);	//LCD led PWM

	lcd_clrscr();
	lcd_puts_hu(PSTR("Fûtés vezérlés v0.9"));

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);

	BUZZER_DDR |= (1 << BUZZER_PIN);

	THERMOSTAT_DDR &= ~((1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN));
	THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);

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
	lcd_puts_hu(PSTR("BME280 indítás"));
	init_BME280();

	sei();
	uart_puts_p(PSTR("Interrupt enabled\n"));


	ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
	nSensors = search_sensors();

	if (Mode == DEBUG)
		{ uart_puts_P("Found "); uart_puti(nSensors); uart_puts_P(" DS18B20 sensors\n"); }
	lcd_gotoxy(0,2);
	lcd_puts_p(PSTR("DS18B20: ")); lcd_puti(nSensors); lcd_puts_p(PSTR(" db"));

	_delay_ms(3000);

	TCCR1B |= (1 << CS12) | (1 << WGM12);
	TCNT1 = 0;
	OCR1A = 62500;//14400; // 1000ms
	TIMSK |= (1 << OCIE1A);

	//ADMUX |= (1<<REFS0) | (1<< REFS1);
	ADCSRA |= (1 << ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);

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
				if (readchar != CHAR_RETURN)
				{
					uartdata_in[uartdata_count++] = readchar;
				}
				else
				{
					uartdata_in[uartdata_count] = CHAR_RETURN;

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
								uart_puts(DHWTempActualBuf); uart_puts_P("."); uart_puts(DHWTempActualFracBuf);
								break;
							case '2':
								uart_puts(BufferTempActualBuf); uart_puts_P("."); uart_puts(BufferTempActualFracBuf);
								break;
							case '3':
								uart_puts(EngineeringTempActualBuf); uart_puts_P("."); uart_puts(EngineeringTempActualFracBuf);
								break;
							case '4':
								uart_puts(GarageTempBuf); uart_puts_P("."); uart_puts(GarageTempFracBuf);
								break;
							case '5':
								uart_puts(LivingRoomTempBuf); uart_puts_P("."); uart_puts(LivingRoomTempFracBuf);
								break;
							case '6':
								uart_puts(ForwardTempBuf); uart_puts_P("."); uart_puts(ForwardTempFracBuf);
								break;
							case '7':
								uart_puts(ReturnTempBuf); uart_puts_P("."); uart_puts(ReturnTempFracBuf);
								break;
							case '8':
								uart_puts(BME280TempBuf);
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
	uart_puts_p(PSTR("Fatal error, program end\n"));
}