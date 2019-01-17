#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

enum states {
	BME280TempState,
	BME280HumidState,
	DS18B20State1,
	DS18B20State2
};

enum daynames {
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY
};

//#define SOLAR
//#define BUFFER

#define UART_BAUD_RATE      9600
#define CHAR_NEWLINE 		'\n'
#define CHAR_RETURN 		'\r'
#define RETURN_NEWLINE 		"\r\n"

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

#define DS18B20_MAX_NO		8
#define DS18B20_PORT		PORTB
#define DS18B20_DDR			DDRB
#define DS18B20_PIN			PINB
#define DS18B20_PINx		PINB0

// extern volatile uint8_t DebugMode; extern uint8_t eeDebugMode;
// extern volatile uint8_t MenuTimer; extern uint8_t eeMenuTimer;
// extern volatile uint8_t LCDBackLight; extern uint8_t eeLCDBackLight;
// extern volatile uint16_t PumpPlusTime; extern uint16_t eePumpPlusTime;
extern uint8_t DebugMode; extern uint8_t eeDebugMode;
extern uint8_t MenuTimer; extern uint8_t eeMenuTimer;
extern uint8_t LCDBackLight; extern uint8_t eeLCDBackLight;
extern uint16_t PumpPlusTime; extern uint16_t eePumpPlusTime;
extern uint8_t ComfortMode; extern uint8_t eeComfortMode;
extern uint16_t ComfortTemp; extern uint16_t eeComfortTemp;
extern uint8_t ComfortForwardTemp; extern uint8_t eeComfortForwardTemp;

extern float BME280Temp, BME280Humid;
extern char BME280TempBuf[8], BME280HumidBuf[6];

extern uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
extern uint16_t DHWMinTime, DHWMaxTime;
extern uint8_t eeDHWTempDesired, eeDHWTempMin;
extern uint16_t eeDHWMinTime, eeDHWMaxTime;
extern char DHWTempActualBuf[4], DHWTempActualFracBuf[3];

extern uint8_t BufferTempActual;
extern uint8_t ForwardHeatTemp;
extern uint8_t eeForwardHeatTemp;
extern char BufferTempActualBuf[4], BufferTempActualFracBuf[3];

extern uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin;
extern uint8_t eeEngineeringTempDesired, eeEngineeringTempMin;
extern char EngineeringTempActualBuf[4], EngineeringTempActualFracBuf[3];

extern uint8_t GarageTemp;
extern char GarageTempBuf[4], GarageTempFracBuf[3];

extern uint16_t LivingRoomTemp;
extern char LivingRoomTempBuf[4], LivingRoomTempFracBuf[3];

extern uint8_t FloorTemp;
extern char FloorTempBuf[4], FloorTempFracBuf[3];

extern uint8_t ForwardTemp;
extern char ForwardTempBuf[4], ForwardTempFracBuf[3];

extern uint8_t ReturnTemp;
extern char ReturnTempBuf[4], ReturnTempFracBuf[3];

extern float BME280TempMin, BME280TempMax;
extern uint8_t DHWTempMinMeasured, DHWTempMax, BufferTempMin, BufferTempMax, EngineeringTempMinMeasured, EngineeringTempMax, GarageTempMin, GarageTempMax, FloorTempMin, FloorTempMax;
extern uint16_t LivingRoomTempMin, LivingRoomTempMax;

extern uint16_t SwitchOnOutdoorTempMin;
extern uint16_t eeSwitchOnOutdoorTempMin;

extern uint8_t DHWSensorID;
extern uint8_t eeDHWSensorID;

extern uint8_t BufferSensorID;
extern uint8_t eeBufferSensorID;

extern uint8_t EngineeringSensorID;
extern uint8_t eeEngineeringSensorID;

extern uint8_t GarageSensorID;
extern uint8_t eeGarageSensorID;

extern uint8_t LivingRoomSensorID;
extern uint8_t eeLivingRoomSensorID;

extern uint8_t FloorSensorID;
extern uint8_t eeFloorSensorID;

extern uint8_t ForwardTempSensorID;
extern uint8_t eeForwardTempSensorID;

extern uint8_t ReturnTempSensorID ;
extern uint8_t eeReturnTempSensorID;

extern unsigned char Relays;

extern uint8_t nSensors;

extern uint8_t Hour, Minute, Seconds;
extern uint8_t ClockInitialized;

extern uint16_t Year;
extern uint8_t Month, Day, DayName;

extern void SwitchRelays();

void beep();
void SensorRead();
void CheckConditions();
void SwitchRelays();

#endif /* MAIN_H_ */