//TODO

// külsõ hõmérés, tiltás -> OK
// szivattyú utánfutás -> OK
// kazánház min / kivánt -> OK
// min < kivánt -> OK
// több kijelzés -> OK?
// óra
// külsõ hõmérõ hiba
// elõzõ / aktuális állapot -> kevesebb frissítés

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
//volatile uint8_t TimerState = 0;

// volatile uint8_t DebugMode = 0; uint8_t EEMEM eeDebugMode = 0;
// volatile uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10;
// volatile uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight = 0;
// volatile uint16_t PumpPlusTime = 5; uint16_t EEMEM eePumpPlusTime = 5;

uint8_t DebugMode = 0; uint8_t EEMEM eeDebugMode = 0;
uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10;
uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight = 0;
uint16_t PumpPlusTime = 5; uint16_t EEMEM eePumpPlusTime = 5;

// volatile float BME280Temp; char BME280TempBuf[6];
// volatile float BME280Humid; char BME280HumidBuf[6];
// volatile uint16_t BME280TempUint;

float BME280Temp; char BME280TempBuf[6];
float BME280Humid; char BME280HumidBuf[6];
uint16_t BME280TempUint;

uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
uint16_t DHWMinTime, DHWMaxTime; 
//volatile uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
//volatile uint16_t DHWMinTime, DHWMaxTime; 
char DHWTempActualBuf[3], DHWTempActualFracBuf[3];
uint8_t EEMEM eeDHWTempDesired = 30;
uint8_t EEMEM eeDHWTempMin = 25;
uint16_t EEMEM eeDHWMinTime = 600;
uint16_t EEMEM eeDHWMaxTime = 2200;

uint8_t BufferTempActual; char BufferTempActualBuf[3], BufferTempActualFracBuf[3];
uint8_t ForwardHeatTemp = 25; uint8_t EEMEM eeForwardHeatTemp = 25;
//volatile uint8_t BufferTempActual; char BufferTempActualBuf[3], BufferTempActualFracBuf[3];
//volatile uint8_t ForwardHeatTemp = 25; uint8_t EEMEM eeForwardHeatTemp = 25;

uint8_t GarageTempActual, GarageTempDesired, GarageTempMin; char GarageTempActualBuf[3], GarageTempActualFracBuf[3];
//volatile uint8_t GarageTempActual, GarageTempDesired, GarageTempMin; char GarageTempActualBuf[3], GarageTempActualFracBuf[3];
uint8_t EEMEM eeGarageTempDesired = 10;
uint8_t EEMEM eeGarageTempMin = 5;

//volatile uint16_t SwitchOnOutdoorTempMin; 
uint16_t SwitchOnOutdoorTempMin;
uint16_t EEMEM eeSwitchOnOutdoorTempMin = 2300;

//volatile unsigned char Relays = 0;
unsigned char Relays = 0;

//volatile uint8_t DHWSensorID = 0;
uint8_t DHWSensorID = 0;
uint8_t EEMEM eeDHWSensorID = 0;

//volatile uint8_t BufferSensorID = 1;
uint8_t BufferSensorID = 1;
uint8_t EEMEM eeBufferSensorID = 1;

//volatile uint8_t GarageSensorID = 2;
uint8_t GarageSensorID = 2;
uint8_t EEMEM eeGarageSensorID = 2;

uint8_t nSensors;
uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];

//volatile uint8_t Hour = 0, Minute = 0, Seconds = 0;
uint8_t Hour = 0, Minute = 0, Seconds = 0;


ISR(ADC_vect) // LCD háttérk világítás PWM
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
	static uint8_t display = 10;
	static uint8_t timerstate = 0;
	
	switch (timerstate)
	{
		case (BME280TempState):
			BME280Temp = bme280_readTemperature();
			ftoa(BME280TempBuf, BME280Temp, 2);
			BME280TempUint = (uint16_t)( BME280Temp * 100);
			if (DebugMode > 0)
				{ uart_puts_p(PSTR("BME280 Temperature: ")); uart_puts(BME280TempBuf); uart_puts_p(PSTR("C \n")); }
			timerstate++;
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
			for ( i=0; i<nSensors; i++ )
			{
				DS18X20_start_meas(DS18X20_POWER_EXTERN,&gSensorIDs[i][0]);
			}
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
				}
				if (i == BufferSensorID)
				{
					BufferTempActual = cel;
					itoa(BufferTempActual, BufferTempActualBuf, 10);
					itoa(cel_frac_bits, BufferTempActualFracBuf, 10);
				}
				if (i == GarageSensorID)
				{
					GarageTempActual = cel;
					itoa(GarageTempActual, GarageTempActualBuf, 10);
					itoa(cel_frac_bits, GarageTempActualFracBuf, 10);
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
		if (display > 5)
		{
			lcd_puts_p(PSTR("HMV ")); lcd_puts(DHWTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(DHWTempActualFracBuf); lcd_puts_p(PSTR(" C")); lcd_gotoxy(15,0); lcd_puti(Hour); lcd_puts_p(PSTR(":")); lcd_puti(Minute);
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("Puffer ")); lcd_puts(BufferTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(BufferTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("Gépház ")); lcd_puts(GarageTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(GarageTempActualFracBuf); lcd_puts_p(PSTR(" C"));
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("Külsõ ")); lcd_puts(BME280TempBuf); lcd_puts_p(PSTR(" C"));
		}
		else if (display <= 5)
		{
			lcd_puts_p(PSTR("HMV szelep ")); lcd_putbit(Relays, DHW_VALVE_RELAY); lcd_gotoxy(15,0); lcd_puti(Hour); lcd_puts_p(PSTR(":")); lcd_puti(Minute);
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("Gáz relé ")); lcd_putbit(Relays, GAS_RELAY);
			lcd_gotoxy(0,2);
			lcd_puts_p(PSTR("Puffer sz/p ")); lcd_putbit(Relays, BUFFER_VALVE_RELAY); lcd_putbit(Relays, BUFFER_PUMP_RELAY);
			lcd_gotoxy(0,3);
			lcd_puts_hu(PSTR("Föld/Emel ")); lcd_putbit(Relays, FIRST_FLOOR_VALVE); lcd_putbit(Relays, SECOND_FLOOR_VALVE);
		}
		if (--display == 0)
			display = 10;
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
	if (DHWTempActual < DHWTempDesired && (DHWMaxTime >= currTime && DHWMinTime <= currTime )) // alacsony HMV hõmérséklet
	{
		if (DHWTempMin >= DHWTempActual)
		{
			if (BufferTempActual < DHWTempDesired) // Pufferben nincs elég energia
			{
				pumpplustime = PumpPlusTime;
				Relays |= (1 << DHW_VALVE_RELAY)|(1 << GAS_RELAY);
				Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
				if (DebugMode > 0)
					uart_puts_P("DHW relay activated with gas\n");
			}
			else // Pufferben van elég energia
			{
				if (!pumpplustime) // gáz megy, de átkapcsol pufferre, gáz még utókeringetne
					Relays |= ((1 << DHW_VALVE_RELAY) | (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
				else
				pumpplustime--;
				Relays &= ~(1 << GAS_RELAY);
				if (DebugMode > 0)
					uart_puts_P("DHW relay activated with buffer\n");
			}
		}
	}
	else // megszûnõ hõigény esetén az utókeringetés a gépház radiátorát fûti
	{
		Relays &= ~((1 << GAS_RELAY) | (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
		if (DebugMode > 0)
			uart_puts_P("DHW relay deactivated\n");
	}

	// fûtési kör
	if (!(Relays & (1 << DHW_VALVE_RELAY)))
	{
		if ((!(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)) || (GarageTempActual < GarageTempDesired))  && (BME280TempUint <= SwitchOnOutdoorTempMin))		
		{
			if (GarageTempActual < GarageTempMin || !(THERMOSTAT_PIN & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PIN & (1 << SECOND_THERMO_PIN)))
			{
				if (BufferTempActual < ForwardHeatTemp) // Pufferben nincs elég energia
				{
					pumpplustime = PumpPlusTime; // gáz utókeringetés
					Relays |= (1 << GAS_RELAY);
					Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
					if (DebugMode > 0)
					uart_puts_P("GAS activated; BUFFER, DHW relay deactivated\n");
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
					
					if (DebugMode > 0)
					uart_puts_P("GAS, DHW deactivated; BUFFER relay deactivated\n");
				}
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

	if (DebugMode < 2)
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
	GarageSensorID = eeprom_read_byte(&eeGarageSensorID);
	GarageTempDesired = eeprom_read_byte(&eeGarageTempDesired);
	GarageTempMin = eeprom_read_byte(&eeGarageTempMin);
	SwitchOnOutdoorTempMin = eeprom_read_word(&eeSwitchOnOutdoorTempMin);
	DebugMode = eeprom_read_byte(&eeDebugMode);
	MenuTimer = eeprom_read_byte(&eeMenuTimer);
	LCDBackLight = eeprom_read_byte(&eeLCDBackLight);
	PumpPlusTime = eeprom_read_word(&eePumpPlusTime);
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
	lcd_puts_hu(PSTR("Fûtés vezérlés v0.8"));

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);

	BUZZER_DDR |= (1 << BUZZER_PIN);

	THERMOSTAT_DDR &= ~((1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN));
	//THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);

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
				Minute++;
				if (Minute == 60)
				{
					Minute = 0;
					Hour++;
					if (Hour == 24)
						Hour = 0;
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