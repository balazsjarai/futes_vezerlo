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
#include "DHT/DHTxx.h"
#include "BMP280/bme280.h"
#include "Utils/ftoa.h"
#include "UART/UART.h"
#include "SPI/spi.h"
#include "DS18B20/crc8.h"
#include "DS18B20/delay.h"
#include "DS18B20/ds18x20.h"
#include "DS18B20/onewire.h"

#define UART_BAUD_RATE      9600

#define RELAYS_1_PIN	PINA1

#define DHW_RELAY 		1
#define SOLAR_RELAY 	2
#define BUFFER_RELAY 	3

#define DHW_PWM_OCR		OCRA2

volatile float bme280_temp; char BME280_temp_buf[5];
volatile float bme280_humid; char BME280_humid_buf[5];
volatile uint8_t menutimer = 10;

volatile uint16_t DHW_temp_actual, DHW_temp_desired, DHW_temp_max, DHW_temp_lowlimit; char DHW_temp_actual_buf[5];
volatile uint16_t solar_temp_actual, solar_temp_desired, solar_temp_up_threshold;
volatile uint16_t buffer_temp_actual;
volatile uint16_t first_termo, second_termo, aux_temp_actual, aux_temp_desired, forward_heat_temp;

volatile char DHW_pump, DHW_PWM, solar_pump, buffer_pump, RELAYED_PUMP;

unsigned char relays_1;
unsigned char DHW_sensor_ID = 0;
unsigned char EEMEM eeDHW_sensor_ID = 0;


ISR(ADC_vect)
{
	if (ADMUX & (1<<MUX0))
	{
		OCR0 = ADC >> 2; //ADC / 4; jobbra shiftelés = osztás néggyel
		ADMUX &= ~(1<<MUX0);
	}
	else
	{
		tempADC = ADC;
		ADMUX |= (1<<MUX0);
	}
	if (OCR0 == 0 || OCR0 == 255)
	OCR0 = 128;
}

ISR(TIMER1_COMPA_vect)
{
	sensor_read();
}

void beep()
{
	TCCR0 |= (1 << CS00)|(1 << CS02);
	OCR0 = 125;
	TIMSK |= (1 << OCIE0);
	BUZZER_PORT |= (1 << BUZZER_PIN);
}

void sensor_read()
{
	static uint8_t timer_state = BMP280_temp;
	char buf[10];
	div_t tempLM;

	switch (timer_state)
	{
		case (BMP280_temp):
			bme280_temp = bme280_readTemperature();
			ftoa(BME280_temp_buf, bme280_temp, 2);
			uart_puts_p(PSTR("BME280 Temperature: ")); uart_puts(BME280_temp_buf); uart_puts_p(PSTR("C \n"));
			timer_state++;
		break;

		case (BMP280_humid):
			bme280_humid = bme280_readHumidity();
			ftoa(BME280_humid_buf, bme280_humid, 2);
			uart_puts_p(PSTR("BME280 Humidity: "));	uart_puts(BME280_humid_buf);	uart_puts_p(PSTR("% \n"));
			timer_state++;
		break;

		case (ADC_state):
			
			timer_state++;
		break;

		case (ADC_state_ex):
		
		
			timer_state++;
		break;

		case (DS18B20):

			timer_state = 0;
		break;

		default:
		timer_state = 0;
		break;
	}

	if (menutimer == 0)
	{
		lcd_clrscr();
		lcd_puts_hu(PSTR("BME280 hõm ")); lcd_puts(BME280_temp_buf); 
		lcd_gotoxy(0,1);
		lcd_puts_hu(PSTR("HMV hõm ")); lcd_puts(DHW_temp_actual_buf);
		
	}
	else
	menutimer--;

	//PORTD ^= (1 << PIND7);

	return;
}


void switch_on_DHW_relay()
{
	relays_1 |= (1 << DHW_RELAY);
	SPI_Write(RELAYS_1_PIN, relays_1);
}

void switch_off_DHW_relay()
{
	relays_1 &= ~(1 << DHW_RELAY);
	SPI_Write(RELAYS_1_PIN, relays_1);
}

void switch_on_PWM_for_DHW_pump(){};
void switch_off_PWM_for_DHW_pump(){};


void valve_solar_on(){};
void pump_solar_on(){};
void pump_solar_off(){};
void valve_buffer_on(){};
void pump_back_on(){};
void switch_on_gas(){};
void switch_on_buffer_heating_pump(){};
void valve_first_on(){};
void valve_second_on(){};

void read_from_eeprom()
{
	DWH_sensor_ID = eeprom_read_byte(&eeDHW_sensor_ID);
}

int main(void){

	tempADC = 0;
	bme280_temp = 0;
	bme280_pres = 0;
	bme280_humid = 0;

	lcd_init(LCD_DISP_ON);
	//menuInit();
	//uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	//ADMUX |= (1<<REFS0) | (1<< REFS1);
	//ADCSRA |= (1 << ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);

	//i2c_init();
	//init_BME280(); //BMP 280

	//InitADCEx();

	//TCCR1B |= (1 << CS12)|(1 << CS10)|(1 << WGM12);
	//TCNT1 = 0;
	//OCR1A = 3600;//14400; // 1000ms
	//TIMSK |= (1 << OCIE1A);

	//MCUCR |= (1 << ISC00);
	//GICR |= (1 << INT0);
	//DDRD |= (1 << PIND2);
	//PORTD |= (1 << PIND2);

	//uart_puts_p(PSTR("Timer initiated\n"));
	sei();
	//uart_puts_p(PSTR("Interrupt enabled\n"));

	//reset watchdog
	//wdt_reset();
	//WDTCR = (1<<WDE);
	//WDTCR = (1<<WDP2)|(1<<WDP1)|(1<<WDP0);


	TCCR0 |= (1 << CS00)|(1 << CS01)|(1 << WGM01)|(1 << WGM00)|(1<<COM01);
	OCR0 = 128;
	DDRB |= (1 << PINB4);
	PORTB |= (1 << PINB4);

	while(1)
	{
		menuPollButtons();

		// DHW felsõ kör
		if (DHW_temp_actual < DHW_temp_lowlimit && (solar_temp_actual < DHW_temp_actual || buffer_temp_actual < DHW_temp_actual))
		{
			if (DHW_pump == RELAYED_PUMP)
			switch_on_DHW_relay();
			else
			{
				switch_on_PWM_for_DHW_pump();
				uint16_t temp_diff = DHW_temp_desired - DHW_temp_actual;
				if (temp_diff > 30)
					DHW_PWM = 20;
				else if (temp_diff > 20)
					DHW_PWM = 60;
				else if (temp_diff > 10)
					DHW_PWM = 150;
				else if (temp_diff > 5)
					DHW_PWM = 220;
				else if (temp_diff > 0)
					DHW_PWM = 240;
			}
		}
		else if (DHW_temp_actual > DHW_temp_desired || solar_temp_actual > DHW_temp_actual || buffer_temp_actual > DHW_temp_actual)
		{
			if (DHW_pump == RELAYED_PUMP)
			switch_off_DHW_relay();
			else
			{
				switch_off_PWM_for_DHW_pump();
			}
		}

		if (solar_temp_actual > solar_temp_up_threshold)
		{
			if (solar_temp_actual > DHW_temp_actual && solar_temp_actual < DHW_temp_max)
			{
				valve_solar_on();
				pump_solar_on();
			}
			else if (solar_temp_actual > DHW_temp_max && buffer_temp_actual < solar_temp_actual)
			{
				valve_buffer_on();
				pump_solar_on();
			}
		}
		else
		{
			pump_solar_off();
			if (DHW_temp_actual < DHW_temp_desired && buffer_temp_actual > DHW_temp_actual)
			{
				pump_back_on();
			}
		}

		if (solar_pump != RELAYED_PUMP)
		{
			uint16_t temp_diff = solar_temp_desired - DHW_temp_actual;
			if (temp_diff > 30)
			DHW_PWM = 20;
			else if (temp_diff > 20)
			DHW_PWM = 60;
			else if (temp_diff > 10)
			DHW_PWM = 150;
			else if (temp_diff > 5)
			DHW_PWM = 220;
			else if (temp_diff > 0)
			DHW_PWM = 240;
		}

		if (first_termo || second_termo || aux_temp_actual < aux_temp_desired)
		{
			if (buffer_temp_actual < forward_heat_temp)
			{
				switch_on_gas();
			}
			else
			{
				switch_on_buffer_heating_pump();
			}
			if (first_termo)
			valve_first_on();
			if (second_termo)
			valve_second_on();
		}
	}
	uart_puts_p(PSTR("Fatal error, program end\n"));
}