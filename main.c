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
#include "MAX31865/max31865.h"
#include "DS18B20/crc8.h"
#include "DS18B20/delay.h"
#include "DS18B20/ds18x20.h"
#include "DS18B20/onewire.h"

volatile uint8_t DebugMode = 0; uint8_t EEMEM eeDebugMode = 0;
volatile uint8_t MenuTimer = 10; uint8_t EEMEM eeMenuTimer = 10; volatile uint8_t menutimer;
volatile uint8_t LCDBackLight = 0; uint8_t EEMEM eeLCDBackLight;

volatile uint8_t Initialized;

volatile float BME280_temp; char BME280_temp_buf[6];
volatile float BME280_humid; char BME280_humid_buf[6];
volatile uint8_t BME280_temp_min = 10; uint8_t EEMEM eeBME280_temp_min = 10;
volatile uint8_t BME280_temp_desired = 15; uint8_t EEMEM eeBME280_temp_desired = 15;

volatile uint8_t DHW_temp_actual, DHW_temp_desired, DHW_temp_max, DHW_temp_min; char DHW_temp_actual_buf[5];
uint8_t EEMEM eeDHW_temp_desired = 60;
uint8_t EEMEM eeDHW_temp_max = 80;
uint8_t EEMEM eeDHW_temp_min = 50;

volatile uint8_t Solar_temp_actual, Solar_temp_desired, Solar_temp_up_threshold;
volatile uint8_t Buffer_temp_actual, Forward_heat_temp;

volatile unsigned char Pump_relays;
volatile unsigned char Valve_relays;
volatile unsigned char Relay_or_PWM; unsigned char EEMEM eeRelay_or_PWM;

volatile uint8_t DHW_sensor_ID;
uint8_t EEMEM eeDHW_sensor_ID = 0;

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
	sensor_read();
	if (DebugMode == 0 && Initialized)
		check_conditions();
	wdt_reset();
}

void sensor_read()
{
	static uint8_t timer_state = BME280_temp_state;
	uint8_t i;
	uint8_t subzero, cel, cel_frac_bits;


	switch (timer_state)
	{
		case (BME280_temp_state):
			BME280_temp = bme280_readTemperature();
			ftoa(BME280_temp_buf, BME280_temp, 2);
			uart_puts_p(PSTR("BME280 Temperature: ")); uart_puts(BME280_temp_buf); uart_puts_p(PSTR("C \n"));
			timer_state++;
		break;

		case (BME280_humid_state):
			BME280_humid = bme280_readHumidity();
			ftoa(BME280_humid_buf, BME280_humid, 2);
			uart_puts_p(PSTR("BME280 Humidity: "));	uart_puts(BME280_humid_buf);	uart_puts_p(PSTR("% \n"));
			
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
			for ( i=0; i<nSensors; i++ )
			{
				if ( DS18X20_read_meas( &gSensorIDs[i][0], &subzero, &cel, &cel_frac_bits) == DS18X20_OK )
				{
					if (i == DHW_sensor_ID)
					{
						uart_puts_P("DHW temperature: ");
						DHW_temp_actual = cel;
					}

					uart_puti(subzero); uart_puti(cel); uart_puts_P("."); uart_puti(cel_frac_bits); uart_puts_P("\n");
				}
			}
			
			timer_state++;
		break;
		
		case (MAX31865_state):
			SPIInit();
			
			char temp1;
			char temp2;
			temp1 = SPIWrite(MAX31865_1_CS_PIN, READ_RTD_LSB);
			temp2 = SPIWrite(MAX31865_1_CS_PIN, 0xFF);
			
			SPIClose();
			
			uart_puts_P("MAX1: ");
			uart_puthex_nibble(temp1); uart_puthex_nibble(temp2);
			uart_puts_P("\n");
			
			timer_state = 0;
			Initialized = 1;
		break;

		default:
			timer_state = 0;
		break;
	}

	if (menutimer == 0)
	{
		lcd_clrscr();
		lcd_puts_hu(PSTR("Kazßnhßz h§m ")); lcd_puts(BME280_temp_buf); 
		lcd_gotoxy(0,1);
		lcd_puts_hu(PSTR("HMV h§m ")); lcd_puts(DHW_temp_actual_buf);		
		lcd_gotoxy(0,2);
		lcd_puts_hu(PSTR("Szelepek: ")); lcd_putbyte_bin(Valve_relays);
		lcd_gotoxy(0,3);
		lcd_puts_hu(PSTR("Pumpßk: ")); lcd_putbyte_bin(Pump_relays);
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
	if (DHW_temp_actual < DHW_temp_min && (Solar_temp_actual < DHW_temp_actual || Buffer_temp_actual < DHW_temp_actual))
	{
		if (!(Relay_or_PWM & (1 << DHW)))
		{
			Pump_relays |= (1 << DHW_RELAY);
		}
		else
		{
			switch_on_PWM_for_DHW_pump();
			uint16_t temp_diff = DHW_temp_desired - DHW_temp_actual;
			if (temp_diff > 30)
				DHW_PWM_OCR = 20;
			else if (temp_diff > 20)
				DHW_PWM_OCR = 60;
			else if (temp_diff > 10)
				DHW_PWM_OCR = 150;
			else if (temp_diff > 5)
				DHW_PWM_OCR = 220;
			else if (temp_diff > 0)
				DHW_PWM_OCR = 240;
		}
	}
	else if (DHW_temp_actual > DHW_temp_desired || Solar_temp_actual > DHW_temp_actual || Buffer_temp_actual > DHW_temp_actual)
	{
		if (!(Relay_or_PWM & (1 << DHW)))
		{
			Pump_relays &= ~(1 << DHW_RELAY);
		}
		else
		{
			switch_off_PWM_for_DHW_pump();
		}
	}

	if (Solar_temp_actual > Solar_temp_up_threshold)
	{
		if (Solar_temp_actual > DHW_temp_actual && Solar_temp_actual < DHW_temp_max)
		{
			Pump_relays |= (1 << SOLAR_RELAY);
			Valve_relays |= (1 << SOLAR_VALVE);			
		}
		else if (Solar_temp_actual > DHW_temp_max && Buffer_temp_actual < Solar_temp_actual)
		{
			Pump_relays |= (1 << SOLAR_RELAY);
			Valve_relays |= (1 << BUFFER_VALVE);
		}
	}
	else
	{
		Pump_relays &= ~(1 << SOLAR_RELAY);
		if (DHW_temp_actual < DHW_temp_desired && Buffer_temp_actual > DHW_temp_actual)
		{
			Pump_relays |= (1 << SOLAR_BACK_RELAY);
		}
	}

	if (!(Relay_or_PWM & (1 << SOLAR)))
	{
		uint16_t temp_diff = Solar_temp_desired - DHW_temp_actual;
		if (temp_diff > 30)
			SOLAR_PWM_OCR = 20;
		else if (temp_diff > 20)
			SOLAR_PWM_OCR = 60;
		else if (temp_diff > 10)
			SOLAR_PWM_OCR = 150;
		else if (temp_diff > 5)
			SOLAR_PWM_OCR = 220;
		else if (temp_diff > 0)
			SOLAR_PWM_OCR = 240;
	}

	if ((THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)) || (THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)) || BME280_temp < BME280_temp_desired)
	{
		if (Buffer_temp_actual < Forward_heat_temp)
		{
			Pump_relays |= (1 << GAS_RELAY);
			Pump_relays &= ~(1 << BUFFER_RELAY);
		}
		else
		{
			Pump_relays &= ~(1 << GAS_RELAY);
			Pump_relays |= (1 << BUFFER_RELAY);
		}
		
		if ((THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)))
			Valve_relays |= (1 << FIRST_FLOOR_VALVE);
		if (!(THERMOSTAT_PORT & (1 << FIRST_THERMO_PIN)))
			Valve_relays &= ~(1 << FIRST_FLOOR_VALVE);
		
		if ((THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)))
			Valve_relays |= (1 << SECOND_FLOOR_VALVE);
		if (!(THERMOSTAT_PORT & (1 << SECOND_THERMO_PIN)))
			Valve_relays &= ~(1 << SECOND_FLOOR_VALVE);
	}
	else
	{
		Pump_relays &= ~((1 << BUFFER_RELAY) | (1 << GAS_RELAY));
		Valve_relays &= ~((1 << BUFFER_VALVE) | (1 << FIRST_FLOOR_VALVE) | (1 << SECOND_FLOOR_VALVE));
	}
	
	SwitchPump();
	SwitchValve();
}

uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;
	
	/* clear display and home cursor */
	uart_puts_P("Bus scanning ...\n");
	
	nSensors = 0;
	
	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && nSensors < DS18B20_MAX_NO;)
	{
		DS18X20_find_sensor( &diff, &id[0] );
		
		if( diff == OW_PRESENCE_ERR ) {
			uart_puts_P("No sensor found\n");
			break;
		}
		
		if( diff == OW_DATA_ERR ) {
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


void switch_on_PWM_for_DHW_pump(){};
void switch_off_PWM_for_DHW_pump(){};

void SwitchValve()
{
	SPIInit();
	SPIWrite(VALVES_CS_PIN, Valve_relays);
	SPIClose();
}

void SwitchPump()
{
	SPIInit();
	SPIWrite(PUMPS_CS_PIN, Pump_relays);
	SPIClose();
}

void read_from_eeprom()
{
	DHW_sensor_ID = eeprom_read_byte(&eeDHW_sensor_ID);
	DHW_temp_desired = eeprom_read_byte(&eeDHW_temp_desired);
	DHW_temp_max = eeprom_read_byte(&eeDHW_temp_max);
	DHW_temp_min = eeprom_read_byte(&eeDHW_temp_min);
	BME280_temp_min = eeprom_read_byte(&eeBME280_temp_min);
	BME280_temp_desired = eeprom_read_byte(&eeBME280_temp_desired);
	Relay_or_PWM = eeprom_read_byte(&eeRelay_or_PWM);
	DebugMode = eeprom_read_byte(&eeDebugMode);
	MenuTimer = eeprom_read_byte(&eeMenuTimer);
	LCDBackLight = eeprom_read_byte(&eeLCDBackLight);
}

int main(void)
{
	read_from_eeprom();
	menutimer = MenuTimer;
	
	lcd_init(LCD_DISP_ON);
	menuInit();
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );

	DDRF &= ~(1 << PINF0); //ADC input
	PORTF &= ~(1 << PINF0);
	
	BUZZER_DDR |= (1 << BUZZER_PIN);
	
	THERMOSTAT_DDR &= ~(1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);
	THERMOSTAT_PORT |= (1 << FIRST_THERMO_PIN)|(1 << SECOND_THERMO_PIN);
	
	i2c_init();
	init_BME280(); //BMP 280

	sei();
	uart_puts_p(PSTR("Interrupt enabled\n"));

	//reset watchdog
	wdt_reset();
	WDTCR |= (1<<WDE) | (1 << WDCE);
	WDTCR |= (1<<WDP2)|(1<<WDP1)|(1<<WDP0);
	
	ow_set_bus(&PINB,&PORTB,&DDRB,PINB0);
	nSensors = search_sensors();
		
	uart_puts_P("Found "); uart_puti(nSensors); uart_puts_P(" DS18B20 sensors\n");

	SPIInit();
	
	DDRA |= (1 << MAX31865_1_CS_PIN);
	SPIWrite(MAX31865_1_CS_PIN, WRITE_CONFIG);
	SPIWrite(MAX31865_1_CS_PIN, 0xD1);
	
	_delay_us(100);
	char temp1;
	char temp2;
	temp1 = SPIWrite(MAX31865_1_CS_PIN, READ_CONFIG);
	temp2 = SPIWrite(MAX31865_1_CS_PIN, 0xFF);
	
	DDRA |= (1 << PINA2);
	_delay_us(1);
	SPIWrite(2, 0x55);
	
	SPIClose();

	uart_puts_P("MAX: ");
	uart_puthex_nibble(temp1); uart_puthex_nibble(temp2);
	uart_puts_P("\n");		

	TCCR3A |= (1 << WGM30)|(1<<COM3C1);
	TCCR3B |= (1 << WGM32)|(1 << CS30)|(1 << CS31);
	OCR3C = 128;
	DDRE |= (1 << PINE5);	//LCD led PWM
	PORTE |= (1 << PINE5);	//LCD led PWM
	
	TCCR1B |= (1 << CS12);
	TCNT1 = 0;
	OCR1A = 62500;//14400; // 1000ms
	TIMSK |= (1 << OCIE1A);

	//ADMUX |= (1<<REFS0) | (1<< REFS1);
	ADCSRA |= (1 << ADEN)|(1<<ADIE)|(1<<ADSC)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
	
	while(1)
	{
		menuPollButtons();
	}
	uart_puts_p(PSTR("Fatal error, program end\n"));
}