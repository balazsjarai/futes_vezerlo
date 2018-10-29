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
volatile uint8_t timer_state = 0;

volatile uint8_t DebugMode = 0; uint8_t EEMEM eeDebugMode = 0;
volatile uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10; volatile uint8_t menutimer = 10;
volatile uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight = 0;

volatile float BME280_temp; char BME280_temp_buf[6];
volatile float BME280_humid; char BME280_humid_buf[6];
volatile uint8_t BME280_temp_min = 10; uint8_t EEMEM eeBME280_temp_min = 10;
volatile uint8_t BME280_temp_desired = 15; uint8_t EEMEM eeBME280_temp_desired = 15;

volatile uint8_t DHW_temp_actual, DHW_temp_desired, DHW_temp_max, DHW_temp_min; char DHW_temp_actual_buf[4], DHW_temp_actual_frac_buf[3];
uint8_t EEMEM eeDHW_temp_desired = 60;
uint8_t EEMEM eeDHW_temp_max = 80;
uint8_t EEMEM eeDHW_temp_min = 50;

volatile uint8_t Buffer_temp_actual; char Buffer_temp_actual_buf[4], Buffer_temp_actual_frac_buf[3];
volatile uint8_t Forward_heat_temp = 35; uint8_t eeForward_heat_temp = 35;

volatile unsigned char Relays = 0;

volatile uint8_t DHW_sensor_ID = 0;
uint8_t EEMEM eeDHW_sensor_ID = 0;

volatile uint8_t Buffer_sensor_ID = 1;
uint8_t EEMEM eeBuffer_sensor_ID = 1;

uint8_t nSensors;
uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];


ISR(ADC_vect)
{
	if (ADC < 10)
		OCR3C = 10;
	else
		OCR3C = ADC;
}

ISR(TIMER1_COMPA_vect)
{
	if (TimerElapsed == 0);
		TimerElapsed++;
	// sensor_read();
	// if (DebugMode == 0 && Initialized)
		// check_conditions();
	// wdt_reset();
}

void sensor_read()
{
	//static uint8_t timer_state = BME280_temp_state;
	uint8_t i;
	uint8_t subzero, cel, cel_frac_bits;


	switch (timer_state)
	{
		case (BME280_temp_state):
			BME280_temp = bme280_readTemperature();
			ftoa(BME280_temp_buf, BME280_temp, 2);
			if (DebugMode > 0)
				{ uart_puts_p(PSTR("BME280 Temperature: ")); uart_puts(BME280_temp_buf); uart_puts_p(PSTR("C \n")); }
			timer_state++;
		break;

		case (BME280_humid_state):
			BME280_humid = bme280_readHumidity();
			ftoa(BME280_humid_buf, BME280_humid, 2);
			if (DebugMode > 0)
				{ uart_puts_p(PSTR("BME280 Humidity: "));	uart_puts(BME280_humid_buf);	uart_puts_p(PSTR("% \n")); }
			timer_state++;
		break;

		case (DS18B20_state1):
			ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
			for ( i=0; i<nSensors; i++ )
			{
				DS18X20_start_meas(DS18X20_POWER_EXTERN,&gSensorIDs[i][0]);
			}
			timer_state++;
		break;

		case (DS18B20_state2):

			ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
			i = 0;
			while (i < nSensors)
			//for ( i=0; i<nSensors; i++ )
			{
				if ( DS18X20_read_meas( &gSensorIDs[i][0], &subzero, &cel, &cel_frac_bits) == DS18X20_OK )
				{
					if (DebugMode > 0)
						{ uart_puti(subzero); uart_puti(cel); uart_puts_P("."); uart_puti(cel_frac_bits); uart_puts_P("\n"); }
				}
				if (i == DHW_sensor_ID)
				{
					DHW_temp_actual = cel;
					itoa(DHW_temp_actual, DHW_temp_actual_buf, 10);
					itoa(cel_frac_bits, DHW_temp_actual_frac_buf, 10);
				}
				if (i == Buffer_sensor_ID)
				{
					Buffer_temp_actual = cel;
					itoa(Buffer_temp_actual, Buffer_temp_actual_buf, 10);
					itoa(cel_frac_bits, Buffer_temp_actual_frac_buf, 10);
				}
				i++;
			}

			timer_state++;
		break;

		default:
			timer_state = 0;
		break;
	}

	if (menutimer == 0)
	{
		lcd_clrscr();
		lcd_puts_hu(PSTR("HMV hom ")); lcd_puts(DHW_temp_actual_buf); lcd_puts("."), lcd_puts(DHW_temp_actual_frac_buf); lcd_puts(" C");
		lcd_gotoxy(0,1);
		lcd_puts_hu(PSTR("Puffer hom: ")); lcd_puts(Buffer_temp_actual_buf); lcd_puts("."), lcd_puts(Buffer_temp_actual_frac_buf); lcd_puts(" C");
		lcd_gotoxy(0,2);
		lcd_puts_hu(PSTR("Kazanhaz hom ")); lcd_puts(BME280_temp_buf); lcd_puts(" C");
		lcd_gotoxy(0,3);
		lcd_puts_hu(PSTR("Relek: ")); lcd_putbyte_bin(Relays);
	}
	else
		menutimer--;

	if (LCDBackLight == 0)
		ADCSRA |= (1<<ADSC);

	return;
}

void check_conditions()
{
	// DHW felsõ kör

	if (DHW_temp_actual <= DHW_temp_min && DHW_temp_actual <= DHW_temp_desired) // alacsony HMV hõmérséklet
	{
		if (Buffer_temp_actual <= DHW_temp_actual) // Pufferben nincs elég energia
		{
			Relays |= (1 << DHW_VALVE_RELAY) | (1 << GAS_RELAY);
			Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY));
			if (DebugMode > 0)
				uart_puts_P("DHW relay activated with gas");
		}
		else // Pufferben van elég energia
		{
			Relays |= (1 << DHW_VALVE_RELAY) | (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY);
			Relays &= ~(1 << GAS_RELAY);
			if (DebugMode > 0)
				uart_puts_P("DHW relay activated with buffer");
		}
	}
	else
	{
		Relays &= ~((1 << GAS_RELAY) | (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
		if (DebugMode > 0)
			uart_puts_P("DHW relay deactivated");
	}


	if ((!(THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)) || !(THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)) || BME280_temp < BME280_temp_desired) && !(Relays & (1 << DHW_VALVE_RELAY)))
	{
		if (Buffer_temp_actual < Forward_heat_temp)
		{
			Relays |= (1 << GAS_RELAY);
			Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY) | (1 << DHW_VALVE_RELAY));
			if (DebugMode > 0)
				uart_puts_P("GAS activated; BUFFER, DHW relay deactivated");
		}
		else
		{
			Relays &= ~((1 << GAS_RELAY) | (1 << DHW_VALVE_RELAY));
			Relays |= (1 << BUFFER_VALVE_RELAY) | (1 << BUFFER_PUMP_RELAY);
			if (DebugMode > 0)
				uart_puts_P("GAS, DHW deactivated; BUFFER relay deactivated");
		}

		if (!(THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)))
		{
			Relays |= (1 << FIRST_FLOOR_VALVE);
			if (DebugMode > 0)
				uart_puts_P("FIRST FLOOR activated");
		}
		if ((THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)))
		{
			Relays &= ~(1 << FIRST_FLOOR_VALVE);
			if (DebugMode > 0)
				uart_puts_P("FIRST FLOOR deactivated");
		}
		if (!(THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)))
		{
			Relays |= (1 << SECOND_FLOOR_VALVE);
			if (DebugMode > 0)
				uart_puts_P("SECOND FLOOR activated");
		}
		if ((THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)))
		{
			Relays &= ~(1 << SECOND_FLOOR_VALVE);
			if (DebugMode > 0)
				uart_puts_P("SECOND FLOOR deactivated");
		}
	}
	else
	{
		Relays &= ~((1 << BUFFER_VALVE_RELAY) | (1 <<BUFFER_PUMP_RELAY) | (1 << GAS_RELAY));
		if (DebugMode > 0)
			uart_puts_P("GAS and FIRST_FLOOR_VALVE, SECOND_FLOOR_VALVE relay deactivated");
	}

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
	DHW_sensor_ID = eeprom_read_byte(&eeDHW_sensor_ID);
	DHW_temp_desired = eeprom_read_byte(&eeDHW_temp_desired);
	DHW_temp_max = eeprom_read_byte(&eeDHW_temp_max);
	DHW_temp_min = eeprom_read_byte(&eeDHW_temp_min);
	Buffer_sensor_ID = eeprom_read_byte(&eeBuffer_sensor_ID);
	Forward_heat_temp = eeprom_read_byte(&eeForward_heat_temp);
	BME280_temp_min = eeprom_read_byte(&eeBME280_temp_min);
	BME280_temp_desired = eeprom_read_byte(&eeBME280_temp_desired);
	DebugMode = eeprom_read_byte(&eeDebugMode);
	MenuTimer = eeprom_read_byte(&eeMenuTimer);
	LCDBackLight = eeprom_read_byte(&eeLCDBackLight);
}

int main(void)
{
	read_from_eeprom();
	menutimer = MenuTimer;

	lcd_init(LCD_DISP_ON);
	lcd_defc(magyar_betuk);

	lcd_clrscr();
	lcd_puts("Futes vezerles v0.8");

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);

	BUZZER_DDR |= (1 << BUZZER_PIN);

	THERMOSTAT_DDR &= ~(1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);
	THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);

	Relays = 0x00;
	SwitchRelays();

	lcd_gotoxy(0,1);
	lcd_puts("Alapallapotba allas");
	lcd_gotoxy(0,2);
	uint8_t init_counter = 0;
	while (init_counter < 10)
	{
		lcd_putc('x');
		_delay_ms(1000);
		init_counter++;
	}

	lcd_clrscr();
	lcd_puts("I2C inditas");
	i2c_init();
	lcd_gotoxy(0,1);
	lcd_puts("BME280 inditas");
	init_BME280();

	sei();
	uart_puts_p(PSTR("Interrupt enabled\n"));

	//reset watchdog
	wdt_reset();
	WDTCR |= (1<<WDE) | (1 << WDCE);
	WDTCR |= (1<<WDP2)|(1<<WDP1)|(1<<WDP0);

	ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
	nSensors = search_sensors();

	if (DebugMode > 0)
		{ uart_puts_P("Found "); uart_puti(nSensors); uart_puts_P(" DS18B20 sensors\n"); }
	lcd_gotoxy(0,2);
	lcd_puts("DS18B20: "); lcd_puti(nSensors); lcd_puts(" db");

	_delay_ms(3000);

	TCCR3A |= (1 << WGM30)|(1<<COM3C1);
	TCCR3B |= (1 << WGM32)|(1 << CS30)|(1 << CS31);
	OCR3C = 128;
	DDRE |= (1 << PINE5);	//LCD led PWM
	PORTE |= (1 << PINE5);	//LCD led PWM

	lcd_gotoxy(0,3);

	TCCR1B |= (1 << CS12);
	TCNT1 = 0;
	OCR1A = 62500;//14400; // 1000ms
	TIMSK |= (1 << OCIE1A);

	//ADMUX |= (1<<REFS0) | (1<< REFS1);
	ADCSRA |= (1 << ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);

	menuInit();

	while(1)
	{
		if (TimerElapsed == 1)
		{
			TimerElapsed++;
			sensor_read();
			if (DebugMode == 0)
				check_conditions();
			wdt_reset();
			TimerElapsed = 0;
		}
		menuPollButtons();
	}
	uart_puts_p(PSTR("Fatal error, program end\n"));
}