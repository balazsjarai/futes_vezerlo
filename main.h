#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

typedef enum {
	BMP280_temp,
	BMP280_humid,
	BMP280_press,
	ADC_state,
	ADC_state_ex,
	DS18B20
} states;

#define BUZZER_PORT	PORTB
#define BUZZER_DDR	DDRB
#define BUZZER_PIN	PB1

void beep();
void sensor_read();
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
void switch_on_buffer_heating_pump();
void valve_first_on();
void valve_second_on();

#endif /* MAIN_H_ */