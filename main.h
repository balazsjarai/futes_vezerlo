#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

typedef enum {
	BME280_temp_state,
	BME280_humid_state,
	DS18B20_state1,
	DS18B20_state2,
	MAX31865_state
} states;

#define UART_BAUD_RATE      9600

#define DHW_RELAY_CS_PIN	1
#define PUMPS_CS_PIN		2
#define VALVES_CS_PIN		3
#define MAX31865_1_CS_PIN	7

#define DHW_RELAY 			1
#define SOLAR_RELAY 		2
#define BUFFER_RELAY 		3
#define GAS_RELAY			4
#define SOLAR_BACK_RELAY	5

#define FIRST_FLOOR_VALVE	1
#define SECOND_FLOOR_VALVE	2
#define SOLAR_VALVE			3
#define BUFFER_VALVE		4

typedef enum {
	DHW,
	BUFFER,
	SOLAR
} PUMP_RELAY_PWM;

#define DHW_PWM_OCR		OCR2
#define SOLAR_PWM_OCR	OCR3A

#define BUZZER_DDR		DDRC
#define BUZZER_PORT		PORTC
#define BUZZER_PIN		PINC0

#define THERMOSTAT_PORT		PORTF
#define THERMOSTAT_DDR		DDRF
#define FIRST_THERMO_PIN	PINF1
#define SECOND_THERMO_PIN	PINF2

#define DS18B20_MAX_NO		6

void beep();
void sensor_read();
void check_conditions();
void switch_on_DHW_relay();
void switch_off_DHW_relay();
void switch_on_PWM_for_DHW_pump();
void switch_off_PWM_for_DHW_pump();
void valve_solar_on();
void pump_solar_on();
void pump_solar_off();
void valve_buffer_on();
void pump_back_on();
void switch_on_gas();
void switch_off_gas();
void switch_on_buffer_heating_pump();
void valve_first_on();
void valve_second_on();

void SwitchValve();
void SwitchPump();

#endif /* MAIN_H_ */