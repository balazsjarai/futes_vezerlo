#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>

enum states {
	BME280TempState,
	BME280HumidState,
	DS18B20State1,
	DS18B20State2,
	ReportState1,
	ReportState2
};

enum daynames {
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6,
	SUNDAY = 7
};

enum modes {
	AUTOMATIC,
	MANUAL,
	DEBUG1
};

//#define SOLAR
//#define BUFFER

#define UART_BAUD_RATE      9600
#define CHAR_NEWLINE 		'\n'
#define CHAR_RETURN 		'\r'
#define RETURN_NEWLINE 		"\r\n"

#define RELAYS_CS_PIN		1

#define FIRST_FLOOR_VALVE	7
#define SECOND_FLOOR_VALVE	6
#define DHW_VALVE_RELAY 	0
#define BUFFER_VALVE_RELAY 	5
#define BUFFER_PUMP_RELAY	4
#define GAS_RELAY			1

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

extern uint8_t Mode; extern uint8_t eeMode;
extern uint8_t MenuTimer; extern uint8_t eeMenuTimer;
extern uint8_t LCDBackLight; extern uint8_t eeLCDBackLight;
extern uint16_t PumpPlusTime; extern uint16_t eePumpPlusTime;
extern uint8_t ComfortMode; extern uint8_t eeComfortMode;
extern uint16_t ComfortTemp; extern uint16_t eeComfortTemp;
extern uint8_t ComfortForwardTemp; extern uint8_t eeComfortForwardTemp;
extern uint16_t ComfortMaxTime; extern uint16_t eeComfortMaxTime;
extern uint16_t ComfortMinTime; extern uint16_t eeComfortMinTime;
extern uint16_t Restarts; extern uint16_t eeRestarts;

extern float BME280Temp, BME280Humid;
extern char BME280TempBuf[8], BME280HumidBuf[6];

extern uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
extern uint16_t DHWMinTime, DHWMaxTime;
extern uint8_t eeDHWTempDesired, eeDHWTempMin;
extern uint16_t eeDHWMinTime, eeDHWMaxTime;
extern char DHWTempActualBuf[4], DHWTempActualFracBuf[2];

extern uint8_t BufferTempActual;
extern uint8_t ForwardHeatTemp; extern uint8_t eeForwardHeatTemp;
extern uint8_t BufferMaxTemp; extern uint8_t eeBufferMaxTemp;
extern char BufferTempActualBuf[4], BufferTempActualFracBuf[2];

extern uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin;
extern uint8_t eeEngineeringTempDesired, eeEngineeringTempMin;
extern char EngineeringTempActualBuf[4], EngineeringTempActualFracBuf[2];

extern uint8_t GarageTemp;
extern char GarageTempBuf[4], GarageTempFracBuf[2];

extern uint16_t LivingRoomTemp;
extern char LivingRoomTempBuf[4], LivingRoomTempFracBuf[2];

extern uint8_t FloorTemp;
extern char FloorTempBuf[4], FloorTempFracBuf[2];

extern uint8_t ForwardTemp;
extern char ForwardTempBuf[4], ForwardTempFracBuf[2];

extern uint8_t ReturnTemp;
extern char ReturnTempBuf[4], ReturnTempFracBuf[2];

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
uint8_t search_sensors(void);
uint8_t GetFracCel(uint8_t cel_frac_bits);

#endif /* MAIN_H_ */

/*
UART codes

\n - új sor
\r - adás vége

Sxx;yyy.zz 	- szenzor érték küldés

R1;xx - eeprom olvasás, byte
R2;xx - eepron olvasás, word
S;x - szenzor olvasás

W1;xx,yyy - eeprom írás, byte
W2;xx,yyyyy - eeprom írás, word
U - eepromból adatok újraolvasása
M;yyy - relék kapcsolása


hmv 1
puffer 5
gépház 0
garázs 6
nappali 3
padló 4
előre 2
vissza 7
*/