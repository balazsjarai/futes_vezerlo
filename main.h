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

//#define SOLAR
//#define BUFFER

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

extern volatile uint8_t DebugMode; extern uint8_t eeDebugMode;
extern volatile uint8_t MenuTimer; extern uint8_t eeMenuTimer;
extern volatile uint8_t LCDBackLight; extern uint8_t eeLCDBackLight;

extern volatile float BME280_temp, BME280_humid;
extern volatile uint8_t BME280_temp_min, BME280_temp_desired;
extern char BME280_temp_buf[6], BME280_humid_buf[6];

extern uint8_t eeBME280_temp_min, eeBME280_temp_desired;

extern volatile uint8_t DHW_temp_actual, DHW_temp_desired, DHW_temp_min;
extern uint8_t eeDHW_temp_desired, eeDHW_temp_min;

extern volatile uint8_t DHW_sensor_ID;
extern uint8_t eeDHW_sensor_ID;

extern volatile uint8_t Solar_temp_actual, Solar_temp_desired, Solar_temp_up_threshold;
extern volatile uint8_t Buffer_temp_actual;

extern volatile unsigned char Pump_relays, Valve_relays, Relay_or_PWM; 
extern unsigned char eeRelay_or_PWM;

extern uint8_t nSensors;

extern void SwitchPump();
extern void SwitchValve();

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