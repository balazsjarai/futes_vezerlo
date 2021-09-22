#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include "DS18B20/onewire.h"

enum states {
	HIHxxxxStartMeaserementState,
	HIHxxxxReadMeasurementState,
	//HIHxxxxReadState,
	DS18B20State1,
	DS18B20State2,
	ReportState1,
	ReportState2,
	ReportState3
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
#define HOUSE_VALVE_RELAY	2
#define WOODPUMP_RELAY		3

#define BUZZER_DDR		DDRC
#define BUZZER_PORT		PORTC
#define BUZZER_PIN		PINC0

#define THERMOSTAT_PORT		PORTE
#define THERMOSTAT_DDR		DDRE
#define THERMOSTAT_PIN		PINE
#define FIRST_THERMO_PIN	PINE6
#define SECOND_THERMO_PIN	PINE7
#define FIRST_REMOTE_PIN	PINE2
#define SECOND_REMOTE_PIN	PIND3

#define DS18B20_MAX_NO		15
#define DS18B20_PORT		PORTB
#define DS18B20_DDR			DDRB
#define DS18B20_PIN			PINB
#define DS18B20_PINx		PINB0

#define OT_ADDRESS			0x09 << 1

extern uint8_t Mode; extern uint8_t eeMode;
extern uint8_t MenuTimer, menutimer; extern uint8_t eeMenuTimer;
extern uint8_t LCDBackLight; extern uint8_t eeLCDBackLight;
extern uint16_t PumpPlusTime; extern uint16_t eePumpPlusTime;
extern uint8_t ComfortMode; extern uint8_t eeComfortMode;
extern uint8_t ComfortTemp, ComfortTempFrac; extern uint8_t eeComfortTemp, eeComfortTempFrac;
extern uint8_t ComfortForwardTemp; extern uint8_t eeComfortForwardTemp;
extern uint8_t ComfortFloorTemp; extern uint8_t eeComfortFloorTemp;
extern uint8_t ComfortMaxTimeHour; extern uint8_t eeComfortMaxTimeHour;
extern uint8_t ComfortMaxTimeMinute; extern uint8_t eeComfortMaxTimeMinute;
extern uint8_t ComfortMinTimeHour; extern uint8_t eeComfortMinTimeHour;
extern uint8_t ComfortMinTimeMinute; extern uint8_t eeComfortMinTimeMinute;
extern uint16_t Restarts; extern uint16_t eeRestarts;

extern float BME280Temp, BME280Humid;
extern char BME280TempBuf[10], BME280HumidBuf[6];

extern float HIHxxxxTemp, HIHxxxxHumid;
extern char HIHxxxxTempBuf[10], HIHxxxxHumidBuf[6];

typedef struct TempSensor
{
	uint8_t ActualTemp;
	uint8_t ActualTempFrac;
	uint8_t MeasuredMinimumTemp;
	uint8_t MeasuredMaximumTemp;
	char ActualTempBuffer[4];
	char ActualTempFracBuffer[2];
	uint8_t SensorID[OW_ROMCODE_SIZE];
} TempSensor;

extern TempSensor DHW, Buffer, Engineering, Garage, LivingRoom, Floor, GasForward, GasReturn, Forward, Return, Mixed, SecondFloor, Wood;

extern uint8_t DHWTempActual, DHWTempDesired, DHWTempMin;
extern uint8_t eeDHWTempDesired, eeDHWTempMin;
extern uint8_t DHWMinTimeHour, DHWMinTimeMinute, DHWMaxTimeHour, DHWMaxTimeMinute;
extern uint8_t eeDHWMinTimeHour, eeDHWMinTimeMinute, eeDHWMaxTimeHour, eeDHWMaxTimeMinute;
extern char DHWTempActualBuf[4], DHWTempActualFracBuf[2];

extern uint8_t BufferTempActual;
extern uint8_t ForwardHeatTemp; extern uint8_t eeForwardHeatTemp;
extern uint8_t BufferMaxTemp; extern uint8_t eeBufferMaxTemp;
extern uint8_t GasForwardHeatTemp; extern uint8_t eeGasForwardHeatTemp;
extern char BufferTempActualBuf[4], BufferTempActualFracBuf[2];

extern uint8_t EngineeringTempActual, EngineeringTempDesired, EngineeringTempMin;
extern uint8_t eeEngineeringTempDesired, eeEngineeringTempMin;
extern char EngineeringTempActualBuf[4], EngineeringTempActualFracBuf[2];

extern uint8_t SecondFloorDesired, SecondFloorFracDesired, SecondFloorThermostatActive, SecondFloorHysteresis, SecondFloorMinTimeHour, SecondFloorMinTimeMinute, SecondFloorMaxTimeHour, SecondFloorMaxTimeMinute;
extern uint8_t eeSecondFloorDesired;
extern uint8_t eeSecondFloorFracDesired;
extern uint8_t eeSecondFloorThermostatActive;
extern uint8_t eeSecondFloorHysteresis;
extern uint8_t eeSecondFloorMinTimeHour;
extern uint8_t eeSecondFloorMinTimeMinute;
extern uint8_t eeSecondFloorMaxTimeHour;
extern uint8_t eeSecondFloorMaxTimeMinute;

extern uint8_t WoodToPut, WoodTempAlert, WoodDelta;
extern uint16_t WoodPressureBefore, WoodPressureAfter, WoodPressureAlert;
extern uint8_t eeWoodToPut, eeWoodTempAlert, eeWoodDelta;
extern uint16_t eeWoodPressureAlert;

//extern uint8_t GarageTemp;
//extern char GarageTempBuf[4], GarageTempFracBuf[2];

//extern uint16_t LivingRoomTemp;
//extern char LivingRoomTempBuf[4], LivingRoomTempFracBuf[2];

//extern uint8_t FloorTemp;
//extern char FloorTempBuf[4], FloorTempFracBuf[2];
//
//extern uint8_t ForwardTemp;
//extern char ForwardTempBuf[4], ForwardTempFracBuf[2];
//
//extern uint8_t ReturnTemp;
//extern char ReturnTempBuf[4], ReturnTempFracBuf[2];

extern float BME280TempMin, BME280TempMax;
extern uint8_t DHWTempMinMeasured, DHWTempMax, BufferTempMin, BufferTempMax, EngineeringTempMinMeasured, EngineeringTempMax, GarageTempMin, GarageTempMax, FloorTempMin, FloorTempMax;
extern uint16_t LivingRoomTempMin, LivingRoomTempMax;

extern int16_t SwitchOnOutdoorTempMin;
extern uint16_t eeSwitchOnOutdoorTempMin;

extern uint8_t SwitchOnOutdoorTemp;
extern uint8_t eeSwitchOnOutdoorTemp;

extern uint8_t eeDHWSensorID[8];

extern uint8_t eeBufferSensorID[8];

extern uint8_t eeEngineeringSensorID[8];

extern uint8_t eeGarageSensorID[8];

extern uint8_t eeLivingRoomSensorID[8];

extern uint8_t eeFloorSensorID[8];

extern uint8_t eeGasForwardTempSensorID[8];

extern uint8_t eeGasReturnTempSensorID[8];

extern uint8_t eeForwardTempSensorID[8];

extern uint8_t eeReturnTempSensorID[8];

extern uint8_t eeMixedTempSensorID[8];

extern uint8_t eeSecondFloorSensorID[8];

extern uint8_t eeWoodSensorID[8];

extern unsigned char Relays;

extern uint8_t nSensors;
extern uint8_t gSensorIDs[DS18B20_MAX_NO][OW_ROMCODE_SIZE];

extern uint8_t Hour, Minute, Seconds;
extern uint8_t ClockInitialized;

extern uint16_t Year;
extern uint8_t Month, Day, DayName;

extern void SwitchRelays();

void beep();
void SensorRead();
void CheckConditions();
void SwitchRelays();
void TurnOTGasOn(uint8_t temp);
void TurnOTGasOff();
uint8_t DHWCheckCondition();
uint8_t EngineeringCondition();
uint8_t SecondFloorCondition();
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