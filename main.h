#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

typedef enum {
	BME280_temp_state,
	BME280_humid_state,
	DS18B20_state1,
	DS18B20_state2
} states;

//#define SOLAR
//#define BUFFER

#define UART_BAUD_RATE      9600

#define RELAYS_CS_PIN		1

#define FIRST_FLOOR_VALVE	2
#define SECOND_FLOOR_VALVE	3
#define DHW_VALVE_RELAY 	4
#define BUFFER_VALVE_RELAY 	5
#define BUFFER_PUMP_RELAY	6
#define GAS_RELAY			7

#define BUZZER_DDR		DDRC
#define BUZZER_PORT		PORTC
#define BUZZER_PIN		PINC0

#define THERMOSTAT_PORT		PORTE
#define THERMOSTAT_DDR		DDRE
#define THERMOSTAT_PIN		PINE
#define FIRST_THERMO_PIN	PINE6
#define SECOND_THERMO_PIN	PINE7

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

extern volatile uint8_t Buffer_sensor_ID;
extern uint8_t eeBuffer_sensor_ID;

extern volatile uint8_t Buffer_temp_actual;
extern volatile uint8_t Forward_heat_temp;
extern uint8_t eeForward_heat_temp;

extern volatile unsigned char Relays;

extern uint8_t nSensors;

extern void SwitchRelays();

void beep();
void sensor_read();
void check_conditions();
void SwitchRelays();

#endif /* MAIN_H_ */