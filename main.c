//TODO

// k�ls� h�m�r�s, tilt�s -> OK
// szivatty� ut�nfut�s -> OK
// kaz�nh�z min / kiv�nt -> OK
// min < kiv�nt -> OK
// t�bb kijelz�s -> OK
// �ra -> bels� id�z�t�vel -> OK
// k�ls� h�m�r� hiba
// el�z� / aktu�lis �llapot -> kevesebb friss�t�s
// d�tum -> OK
// f�t�s napk�zben, ha elegend� a puffer energi�ja, de termoszt�t nem k�r -> OK
// szabads�g / t�voll�t be�ll�t�s
// OpenTherm
// termoszt�t funkci�
// oszt� termoszelep vez�rl�s
// min/max h�m�rs�klet regisztr�l�s -> OK

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

volatile uint8_t TimerElapsed = 0;

uint8_t DebugMode = 0; uint8_t EEMEM eeDebugMode = 0;
uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10;
uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight = 0;
uint16_t PumpPlusTime = 2; uint16_t EEMEM eePumpPlusTime = 2;
uint8_t ComfortMode = 0; uint8_t EEMEM eeComfortMode = 0;
uint8_t ComfortTemp = 20; uint8_t EEMEM eeComfortTemp = 20;
uint8_t ComfortForwardTemp = 28; uint8_t EEMEM eeComfortForwardTemp = 28;

float BME280Temp; char BME280TempBuf[8];
float BME280Humid; char BME280HumidBuf[6];
int16_t BME280TempInt;

uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
uint16_t DHWMinTime, DHWMaxTime;

char DHWTempActualBuf[3], DHWTempActualFracBuf[3];
uint8_t EEMEM eeDHWTempDesired = 50;
uint8_t EEMEM eeDHWTempMin = 40;
uint16_t EEMEM eeDHWMinTime = 1500;
uint16_t EEMEM eeDHWMaxTime = 2200;

uint8_t BufferTempActual; char BufferTempActualBuf[3], BufferTempActualFracBuf[3];
uint8_t ForwardHeatTemp = 42;
uint8_t EEMEM eeForwardHeatTemp = 42;

uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin; char EngineeringTempActualBuf[3], EngineeringTempActualFracBuf[3];
uint8_t EEMEM eeEngineeringTempDesired = 10;
uint8_t EEMEM eeEngineeringTempMin = 5;

uint8_t GarageTemp;
char GarageTempBuf[3], GarageTempFracBuf[3];

uint8_t LivingRoomTemp;
char LivingRoomTempBuf[3], LivingRoomTempFracBuf[3];

uint8_t FloorTemp;
char FloorTempBuf[3], FloorTempFracBuf[3];

float BME280TempMin, BME280TempMax;
uint8_t DHWTempMinMeasured, DHWTempMax, BufferTempMin, BufferTempMax, EngineeringTempMin, EngineeringTempMax, GarageTempMin, GarageTempMax, LivingRoomTempMin, LivingRoomTempMax, FloorTempMin, FloorTempMax;

uint16_t SwitchOnOutdoorTempMin;
uint16_t EEMEM eeSwitchOnOutdoorTempMin = 2300;

unsigned char Relays = 0;

uint8_t DHWSensorID = 0;
uint8_t EEMEM eeDHWSensorID = 0;

uint8_t BufferSensorID = 1;
uint8_t EEMEM eeBufferSensorID = 1;

uint8_t GarageSensorID = 2;
uint8_t EEMEM eeGarageSensorID = 2;

uint8_t EngineeringSensorID = 3;
uint8_t EEMEM eeEngineeringSensorID = 3;

uint8_t LivingRoomSensorID = 4;
uint8_t EEMEM eeLivingRoomSensorID = 4;

uint8_t FloorSensorID = 5;
uint8_t EEMEM eeFloorSensorID = 5;

uint8_t nSensors;
uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];

uint8_t Hour = 0, Minute = 0, Seconds = 0;
uint8_t ClockInitialized = 0;

uint16_t Year = 2018;
uint8_t Month = 1, Day = 1, DayName = MONDAY;


ISR(ADC_vect) // LCD h�tt�r vil�g�t�s PWM
{
	if (ADC < 10)
		OCR3C = 10;
	else
		OCR3C = ADC;
}

ISR(TIMER1_COMPA_vect) // 1 mp-es id�z�t�
{
	if (TimerElapsed == 0);
		TimerElapsed++;
}

void SensorRead()
{
	//static uint8_t timer_state = BME280_temp_state;
	uint8_t i;
	uint8_t subzero, cel, cel_frac_bits;
	static uint8_t display = 15;
	static uint8_t timerstate = 0;

	switch (timerstate)
	{
		case (BME280TempState):
			BME280Temp = bme280_readTemperature();
			ftoa(BME280TempBuf, BME280Temp, 2);
			//dtostrf(BME280Temp, 4, 2, BME280TempBuf);
			BME280TempInt = (int16_t)( BME280Temp * 100);
			if (DebugMode > 0)
				{ uart_puts_p(PSTR("BME280 Temperature: ")); uart_puts(BME280TempBuf); uart_puts_p(PSTR("C \n")); }
			timerstate++;
			if (BME280Temp > BME280TempMax)
				BME280TempMax = BME280Temp;
			if (BME280Temp < BME280TempMin)
				BME280TempMin = BME280Temp;
		break;

		case (BME280HumidState):
			BME280Humid = bme280_readHumidity();
			ftoa(BME280HumidBuf, BME280Humid, 2);
			if (DebugMode > 0)
				{ uart_puts_p(PSTR("BME280 Humidity: "));	uart_puts(BME280HumidBuf);	uart_puts_p(PSTR("% \n")); }
			timerstate++;
		break;

		case (DS18B20State1):
			ow_set_bus(&DS18B20_PIN,&DS18B20_PORT,&DS18B20_DDR,DS18B20_PINx);
			//for ( i=0; i<nSensors; i++ )
			//{
			//	DS18X20_start_meas(DS18X20_POWER_EXTERN,&gSensorIDs[i][0]);
			//}
			DS18X20_start_meas(DS18X20_POWER_EXTERN,NULL);
			timerstate++;
		break;

		case (DS18B20State2):

			ow_set_bus(&DS18B20_PIN,&DS18B20_PORT,&DS18B20_DDR,DS18B20_PINx);
			i = 0;
			while (i < nSensors)
			//for ( i=0; i<nSensors; i++ )
			{
				if ( DS18X20_read_meas( &gSensorIDs[i][0], &subzero, &cel, &cel_frac_bits) == DS18X20_OK )
				{
					if (DebugMode > 0)
						{ uart_puti(subzero); uart_puti(cel); uart_puts_P("."); uart_puti(cel_frac_bits); uart_puts_P("\n"); }
				}
				if (i == DHWSensorID)
				{
					DHWTempActual = cel;
					itoa(DHWTempActual, DHWTempActualBuf, 10);
					itoa(cel_frac_bits, DHWTempActualFracBuf, 10);
					if (DHWTempActual > DHWTempMax)
						DHWTempActual = DHWTempMax;
					if (DHWTempActual < DHWTempMinMeasured)
						DHWTempActual = DHWTempMinMeasured;
				}
				else if (i == BufferSensorID)
				{
					BufferTempActual = cel;
					itoa(BufferTempActual, BufferTempActualBuf, 10);
					itoa(cel_frac_bits, BufferTempActualFracBuf, 10);
					if (BufferTempActual > BufferTempMax)
						BufferTempActual = BufferTempMax;
					if (BufferTempActual < BufferTempMin)
						BufferTempActual = BufferTempMin;
				}
				else if (i == EngineeringSensorID)
				{
					EngineeringTempActual = cel;
					itoa(EngineeringTempActual, EngineeringTempActualBuf, 10);
					itoa(cel_frac_bits, EngineeringTempActualFracBuf, 10);
					if (EngineeringTempActual > EngineeringTempMax)
						EngineeringTempActual = EngineeringTempMax;
					if (EngineeringTempActual < EngineeringTempMin)
						EngineeringTempActual = EngineeringTempMin;
				}
				else if (i == GarageSensorID)
				{
					GarageTemp = cel;
					itoa(GarageTemp, GarageTempBuf, 10);
					itoa(cel_frac_bits, GarageTempFracBuf, 10);
					if (GarageTemp > GarageTempMax)
						GarageTemp = GarageTempMax;
					if (GarageTemp < GarageTempMin)
						GarageTemp = GarageTempMin;
				}
				else if (i == LivingRoomSensorID)
				{
					LivingRoomTemp = cel;
					itoa(LivingRoomTemp, LivingRoomTempBuf, 10);
					itoa(cel_frac_bits, LivingRoomTempFracBuf, 10);
					if (LivingRoomTemp > LivingRoomTempMax)
						LivingRoomTemp = LivingRoomTempMax;
					if (LivingRoomTemp < LivingRoomTempMin)
						LivingRoomTemp = LivingRoomTempMin;
				}
				else if (i == FloorSensorID)
				{
					FloorTemp = cel;
					itoa(FloorTemp, FloorTempBuf, 10);
					itoa(cel_frac_bits, FloorTempFracBuf, 10);
					if (FloorTemp > FloorTempMax)
						FloorTemp = FloorTempMax;
					if (FloorTemp < FloorTempMin)
						FloorTemp = FloorTempMin;
				}
				i++;
			}

			timerstate++;
		break;

		default:
			timerstate = 0;
		break;
	}

	if (MenuTimer == 0)
	{
		lcd_clrscr();
		if (display > 10)
		{
			lcd_puts_p(PSTR("HMV ")); lcd_puts(DHWTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(DHWTempActualFracBuf); lcd_puts_p(PSTR(" C")); lcd_gotoxy(15,0); lcd_puti(Hour); lcd_puts_p(PSTR(":")); lcd_puti(Minute);
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Puffer ")); lcd_puts(BufferTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(BufferTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("G�ph�z ")); lcd_puts(EngineeringTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(EngineeringTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("K�ls� ")); lcd_puts(BME280TempBuf); lcd_puts_p(PSTR(" C"));
		}
		else if (display <= 10 && display > 5)
		{
			lcd_puts_hu(PSTR("Gar�zs ")); lcd_puts(GarageTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(GarageTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Nappali ")); lcd_puts(LivingRoomTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(LivingRoomTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Padl� ")); lcd_puts(FloorTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(FloorTempFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,3);
			lcd_puti(Year); lcd_puts_p(PSTR(".")); lcd_puti(Month); lcd_puts_p(PSTR(".")); lcd_puti(Day);
			lcd_gotoxy(12,3);
			switch (DayName)
			{
				case 1:
					lcd_puts_hu(PSTR("h�tf�"));
					break;
				case 2:
					lcd_puts_hu(PSTR("kedd"));
					break;
				case 3:
					lcd_puts_hu(PSTR("szerda"));
					break;
				case 4:
					lcd_puts_hu(PSTR("cs�t"));
					break;
				case 5:
					lcd_puts_hu(PSTR("p�ntek"));
					break;
				case 6:
					lcd_puts_hu(PSTR("szombat"));
					break;
				case 7:
					lcd_puts_hu(PSTR("vas�rnap"));
					break;
			}
		}
		else if (display <= 5)
		{
			lcd_puts_p(PSTR("HMV szelep ")); lcd_putbit(Relays, DHW_VALVE_RELAY); lcd_gotoxy(15,0); lcd_puti(Hour); lcd_puts_p(PSTR(":")); lcd_puti(Minute);
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("G�z rel� ")); lcd_putbit(Relays, GAS_RELAY);
			lcd_gotoxy(0,2);
			lcd_puts_p(PSTR("Puffer sz/p ")); lcd_putbit(Relays, BUFFER_VALVE_RELAY); lcd_putbit(Relays, BUFFER_PUMP_RELAY);
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("F�ld/Emel ")); lcd_putbit(Relays, FIRST_FLOOR_VALVE); lcd_putbit(Relays, SECOND_FLOOR_VALVE);
		}
		if (--display == 0)
			display = 15;
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
	// DHW fels� k�r
	static uint16_t pumpplustime = 0;
	uint16_t currTime = Hour * 100 + Minute;
	uint8_t DHW_condition;
	uint8_t _relays = Relays;
	
	if (ClockInitialized)
		DHW_condition = (DHWTempActual < DHWTempDesired && (DHWMaxTime >= currTime && DHWMinTime <= currTime )) ? 1 : 0;
	else
		DHW_condition = (DHWTempActual < DHWTempDesired) ? 1 : 0;

	if (DHW_condition) // alacsony HMV h�m�rs�klet
	{
		if (DHWTempMin >= DHWTempActual)
		{
			if (BufferTempActual < DHWTempDesired) // Pufferben nincs el�g energia
			{
				pumpplustime = PumpPlusTime;
				Relays |= (1 << DHW_VALVE_RELAY)|(1 << GAS_RELAY);
				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
				if (DebugMode > 0)
					uart_puts_P("DHW relay activated with gas\n");
			}
			else // Pufferben van el�g energia
			{
				if (pumpplustime > 0)
					pumpplustime--;
				Relays &= ~(1 << GAS_RELAY);
				if (DebugMode > 0)
					uart_puts_P("DHW relay activated with buffer\n");
			}
		}
	}
	else // megsz�n� h�ig�ny eset�n az ut�keringet�s a g�ph�z radi�tor�t f�ti, k�l�nben visszah�ten� a HMV-t
	{
		Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
		if (DebugMode > 0)
			uart_puts_P("DHW relay deactivated\n");
	}

	// f�t�si k�r
	if (!(Relays & (1 << DHW_VALVE_RELAY)))
	{
		if ((!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) 
			|| (EngineeringTempActual < EngineeringTempDesired) 
			|| (ComfortMode && LivingRoomTemp < ComfortTemp && BufferTempActual > ComfortForwardTemp)) 
			&& (BME280TempInt <= SwitchOnOutdoorTempMin))
		{
			if (EngineeringTempActual < EngineeringTempMin 
				|| !(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
			{
				if (BufferTempActual < ForwardHeatTemp) // Pufferben nincs el�g energia
				{
					pumpplustime = PumpPlusTime; // g�z ut�keringet�s
					Relays |= (1 << GAS_RELAY);
					Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
					if (DebugMode > 0)
						uart_puts_P("GAS activated; BUFFER, DHW relay deactivated\n");
				}
				else // Puffer el�g meleg
				{
					if (!pumpplustime) // ha �tkapcsol g�zr�l pufferre, a g�z m�g ut�keringetne
					{
						Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
						Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
					}
					else
						pumpplustime--;

					if (DebugMode > 0)
					uart_puts_P("GAS, DHW deactivated; BUFFER relay deactivated\n");
				}
			}
			else if (LivingRoomTemp < ComfortTemp && BufferTempActual > ComfortForwardTemp && (THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) && (THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
			{
				if (!pumpplustime) // ha �tkapcsol g�zr�l pufferre, a g�z m�g ut�keringetne
				{
					Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
					Relays |= ((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
				}
				else
					pumpplustime--;
			}

			if (!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)))
			{
				Relays |= (1 << FIRST_FLOOR_VALVE);
				if (DebugMode > 0)
					uart_puts_P("FIRST FLOOR activated\n");
			}
			if (THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN))
			{
				Relays &= ~(1 << FIRST_FLOOR_VALVE);
				if (DebugMode > 0)
					uart_puts_P("FIRST FLOOR deactivated\n");
			}
			if (!(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
			{
				Relays |= (1 << SECOND_FLOOR_VALVE);
				if (DebugMode > 0)
					uart_puts_P("SECOND FLOOR activated\n");
			}
			if (THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN))
			{
				Relays &= ~(1 << SECOND_FLOOR_VALVE);
				if (DebugMode > 0)
					uart_puts_P("SECOND FLOOR deactivated\n");
			}
			
			if (ComfortMode)
			{
				if (LivingRoomTemp < ComfortTemp && BufferTempActual > ComfortForwardTemp)
					Relays |= (1 << FIRST_FLOOR_VALVE);
				else
					Relays &= ~(1 << FIRST_FLOOR_VALVE);
			}
			
		}
		else
		{
			if (!pumpplustime)
				Relays &= ~((1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE));
			else
				pumpplustime--;

			Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 <<BUFFER_PUMP_RELAY) | (1 << GAS_RELAY));
			if (DebugMode > 0)
				uart_puts_P("GAS and FIRST_FLOOR_VALVE, SECOND_FLOOR_VALVE relay deactivated\n");
		}
	}

	if (DebugMode < 2 && _relays != Relays)
		SwitchRelays();
}

uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;

	/* clear display and home cursor */
	if (DebugMode > 0)
		uart_puts_P("Bus scanning ...\n");

	nSensors = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && nSensors < DS18B20_MAX_NO;)
	{
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			if (DebugMode > 0)
				uart_puts_P("No sensor found\n");
			break;
		}

		if( diff == OW_DATA_ERR ) {
			if (DebugMode > 0)
				uart_puts_P("Bus error\n");
			break;
		}

		for (i=0;i<OW_ROMCODE_SIZE;i++)
		{
			gSensorIDs[nSensors][i]=id[i];
			//uart_puts_P("ID: "); uart_puti(id[i]); uart_puts_P("\n");
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
	EngineeringSensorID = eeprom_read_byte(&eeEngineeringSensorID);
	EngineeringTempDesired = eeprom_read_byte(&eeEngineeringTempDesired);
	EngineeringTempMin = eeprom_read_byte(&eeEngineeringTempMin);
	GarageSensorID = eeprom_read_byte(&eeGarageSensorID);
	LivingRoomSensorID = eeprom_read_byte(&eeLivingRoomSensorID);
	SwitchOnOutdoorTempMin = eeprom_read_word(&eeSwitchOnOutdoorTempMin);
	DebugMode = eeprom_read_byte(&eeDebugMode);
	MenuTimer = eeprom_read_byte(&eeMenuTimer);
	LCDBackLight = eeprom_read_byte(&eeLCDBackLight);
	PumpPlusTime = eeprom_read_word(&eePumpPlusTime);
	ComfortMode = eeprom_read_byte(&eeComfortMode);
	ComfortTemp = eeprom_read_byte(&eeComfortTemp);
	ComfortForwardTemp = eeprom_read_byte(&eeComfortForwardTemp);
}

int main(void)
{
	read_from_eeprom();

	lcd_init(LCD_DISP_ON);
	lcd_defc(magyar_betuk);

	TCCR3A |= (1 << WGM30)|(1<<COM3C1);
	TCCR3B |= (1 << WGM32)|(1 << CS30)|(1 << CS31);
	OCR3C = 128;
	DDRE |= (1 << PINE5);	//LCD led PWM
	PORTE |= (1 << PINE5);	//LCD led PWM

	lcd_clrscr();
	lcd_puts_hu(PSTR("F�t�s vez�rl�s v0.9"));

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);

	BUZZER_DDR |= (1 << BUZZER_PIN);

	THERMOSTAT_DDR &= ~((1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN));
	//THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);

	Relays = 0x00;
	SwitchRelays();

	lcd_gotoxy(0,1);
	lcd_puts_hu(PSTR("Alap�llapotba �ll�s"));
	lcd_gotoxy(0,2);
	uint8_t init_counter = 0;
	while (init_counter < 10)
	{
		lcd_putc('x');
		_delay_ms(1000);
		init_counter++;
	}

	lcd_clrscr();
	lcd_puts_hu(PSTR("I2C ind�t�s"));
	i2c_init();
	lcd_gotoxy(0,1);
	lcd_puts_hu(PSTR("BME280 ind�t�s"));
	init_BME280();

	sei();
	uart_puts_p(PSTR("Interrupt enabled\n"));


	ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
	nSensors = search_sensors();

	if (DebugMode > 0)
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
			if (DebugMode < 2)
				CheckConditions();
			wdt_reset();
			TimerElapsed = 0;
		}
		menuPollButtons();
	}
	uart_puts_p(PSTR("Fatal error, program end\n"));
}