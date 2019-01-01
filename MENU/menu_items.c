#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "../LCD/lcd.h"
#include "menu.h"
#include "menu_items.h"
#include "../Utils/ftoa.h"
#include "../main.h"
#include "avr/eeprom.h"
#include "../main.h"

/*************************************************************************
 BME 280, submenu definitions
*************************************************************************/
void BME280Temp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280TempBuf);
	
	//char buffer[7];
	//lcd_gotoxy(0,2);
	//ftoa(buffer, BME280TempMin, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	//ftoa(buffer, BME280TempMax, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool BME280Temp_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

void BME280Humidity_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 pára"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280HumidBuf);
}

bool BME280Humidity_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

void SwitchOnOutdoorTempMin_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Külsõ min hõm"));
	lcd_gotoxy(0,1);
	char buf[5];
	itoa(SwitchOnOutdoorTempMin, buf, 10);
	lcd_puts(buf);
}

bool SwitchOnOutdoorTempMin_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (SwitchOnOutdoorTempMin > 3490)
				SwitchOnOutdoorTempMin = 310;
			else
				SwitchOnOutdoorTempMin += 10;
			break;
		case MENU_DOWN:
			if (SwitchOnOutdoorTempMin > 310)
				SwitchOnOutdoorTempMin -= 10;
			else
				SwitchOnOutdoorTempMin = 3500;
			break;
		case MENU_CONFIRM:
			eeprom_update_word(&eeSwitchOnOutdoorTempMin, SwitchOnOutdoorTempMin);
			return true;
		case MENU_CANCEL:
			return true;
	}

	SwitchOnOutdoorTempMin_CallbackRender(column);
	return false;
}

# define BME280_SUBMENU_ITEMS  3
static MENU_ITEM BME280_submenu[BME280_SUBMENU_ITEMS] = {
	{"Külsõ hõm",			BME280Temp_CallbackRender,				BME280Temp_Callback, 				0, NULL},
	{"Külsõ pára",			BME280Humidity_CallbackRender,			BME280Humidity_Callback,			0, NULL},
	{"Külsõ hõm tiltás",	SwitchOnOutdoorTempMin_CallbackRender,	SwitchOnOutdoorTempMin_ActionCallback,	0, NULL},
};


/*************************************************************************
 Menu DHW, submenu definitions
*************************************************************************/
void DHWTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(DHWTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(DHWTempActualFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(DHWTempMinMeasured, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(DHWTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool DHWTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

void DHWTempDesired_CallbackRender(uint8_t which){
	char buf[4];
	itoa(DHWTempDesired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV kívánt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool DHWTempDesired_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (DHWTempDesired != 90)
				++DHWTempDesired;
			break;
		case MENU_DOWN:
			if (DHWTempDesired > DHWTempMin)
				--DHWTempDesired;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHWTempDesired, DHWTempDesired);
			return true;
		case MENU_CANCEL:
			return true;
	}
	DHWTempDesired_CallbackRender(column);
	return false;
}

void DHWTempMin_CallbackRender(uint8_t which){
	char buf[4];
	itoa(DHWTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV min hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool DHWTempMin_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (DHWTempMin < DHWTempDesired)
				++DHWTempMin;
			break;
		case MENU_DOWN:
			if (DHWTempMin > 5)
				--DHWTempMin;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHWTempMin, DHWTempMin);
			return true;
		case MENU_CANCEL:
			return true;
	}

	DHWTempMin_CallbackRender(column);
	return false;
}

void DHWSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(DHWSensorID, buf, 10);
	lcd_puts(buf);
}

bool DHWSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			DHWSensorID++;
			if (DHWSensorID == nSensors)
				DHWSensorID = 0;
			break;
		case MENU_DOWN:
			DHWSensorID--;
			if (DHWSensorID == 255)
				DHWSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHWSensorID, DHWSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	DHWSensor_CallbackRender(column);
	return false;
}

void DHWMinTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Min idõ"));
	lcd_gotoxy(0,1);
	char buf[5];
	itoa(DHWMinTime, buf, 10);
	lcd_puts(buf);
}

bool DHWMinTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (DHWMinTime < 2340)
				DHWMinTime += 10;
			else
				DHWMinTime = 0;
			break;
		case MENU_DOWN:
			if (DHWMinTime > 65525)
				DHWMinTime = 2350;
			else
				DHWMinTime -= 10;
			break;
		case MENU_CONFIRM:
		eeprom_update_word(&eeDHWMinTime, DHWMinTime);
		case MENU_CANCEL:
		return true;
	}

	DHWMinTime_CallbackRender(column);
	return false;
}

void DHWMaxTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Max idõ"));
	lcd_gotoxy(0,1);
	char buf[5];
	itoa(DHWMaxTime, buf, 10);
	lcd_puts(buf);
}

bool DHWMaxTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (DHWMaxTime < 2340)
				DHWMaxTime += 10;
			else
				DHWMaxTime = 0;
			break;
		case MENU_DOWN:
			if (DHWMaxTime > 65525)
				DHWMaxTime = 2350;
			else
				DHWMaxTime -= 10;
			break;
		case MENU_CONFIRM:
		eeprom_update_word(&eeDHWMaxTime, DHWMaxTime);
		case MENU_CANCEL:
		return true;
	}

	DHWMaxTime_CallbackRender(column);
	return false;
}


#define DHWHOURS_SUBMENU_ITEMS 2
static MENU_ITEM DHWHOURS_submenu[DHWHOURS_SUBMENU_ITEMS] = {
	{"Min idõ", 			DHWMinTime_CallbackRender, 		DHWMinTime_ActionCallback,		0,	NULL},
	{"Max idõ", 			DHWMaxTime_CallbackRender, 		DHWMaxTime_ActionCallback,		0,	NULL},
};

# define DHW_SUBMENU_ITEMS  5
static MENU_ITEM DHW_submenu[DHW_SUBMENU_ITEMS] = {
	{"HMV akt hõm", 		DHWTempActual_CallbackRender, 	DHWTempActual_Callback, 		0,						NULL},
	{"HMV kívánt hõm",		DHWTempDesired_CallbackRender,	DHWTempDesired_ActionCallback,	0,						NULL},
	{"HMV min hõm",			DHWTempMin_CallbackRender,  	DHWTempMin_ActionCallback, 		0, 						NULL},
	{"HMV órák",			NULL,							NULL,							DHWHOURS_SUBMENU_ITEMS,	DHWHOURS_submenu},
	{"HMV szenzor",			DHWSensor_CallbackRender, 		DHWSensor_ActionCallback, 		0, 						NULL},
};

/*************************************************************************
 Menu Buffer, submenu definitions
*************************************************************************/
void BufferTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(BufferTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(BufferTempActualFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(BufferTempMin, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(BufferTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool BufferTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

void ForwardHeatTemp_CallbackRender(uint8_t which){
	char buf[4];
	itoa(ForwardHeatTemp, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Elõremenõ hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool ForwardHeatTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (ForwardHeatTemp != 90)
				++ForwardHeatTemp;
			break;
		case MENU_DOWN:
			if (ForwardHeatTemp > 20)
				--ForwardHeatTemp;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeForwardHeatTemp, ForwardHeatTemp);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ForwardHeatTemp_CallbackRender(column);
	return false;
}

void BufferSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(BufferSensorID, buf, 10);
	lcd_puts(buf);
}

bool BufferSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			BufferSensorID++;
			if (BufferSensorID == nSensors)
				BufferSensorID = 0;
			break;
		case MENU_DOWN:
			BufferSensorID--;
			if (BufferSensorID == 255)
				BufferSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeBufferSensorID, BufferSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	BufferSensor_CallbackRender(column);
	return false;
}

# define BUFFER_SUBMENU_ITEMS  3
static MENU_ITEM BUFFER_submenu[BUFFER_SUBMENU_ITEMS] = {
	{"Buffer akt hõm", 		BufferTempActual_CallbackRender, 	BufferTempActual_Callback, 			0,				NULL},
	{"Elõremenõ hõm", 		ForwardHeatTemp_CallbackRender, 	ForwardHeatTemp_ActionCallback,		0,				NULL},
	{"Puffer szenzor",		BufferSensor_CallbackRender, 		BufferSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Garage, submenu definitions
*************************************************************************/
void EngineeringTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(EngineeringTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(EngineeringTempActualFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(EngineeringTempMinMeasured, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(EngineeringTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool EngineeringTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

void EngineeringTempMin_CallbackRender(uint8_t which){
	char buf[4];
	itoa(EngineeringTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház min hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool EngineeringTempMin_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (EngineeringTempMin < EngineeringTempDesired)
				++EngineeringTempMin;
			break;
		case MENU_DOWN:
			if (EngineeringTempMin > 5)
				--EngineeringTempMin;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeEngineeringTempMin, EngineeringTempMin);
			return true;
		case MENU_CANCEL:
			return true;
	}

	EngineeringTempMin_CallbackRender(column);
	return false;
}

void EngineeringTempDesired_CallbackRender(uint8_t which){
	char buf[7];
	itoa(EngineeringTempDesired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház kívánt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool EngineeringTempDesired_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (EngineeringTempDesired < 35)
				++EngineeringTempDesired;
			break;
		case MENU_DOWN:
			if (EngineeringTempDesired > EngineeringTempMin)
				--EngineeringTempDesired;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeEngineeringTempDesired, EngineeringTempDesired);
			return true;
		case MENU_CANCEL:
			return true;
	}
	EngineeringTempDesired_CallbackRender(column);
	return false;
}

void EngineeringSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(EngineeringSensorID, buf, 10);
	lcd_puts(buf);
}

bool EngineeringSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			EngineeringSensorID++;
			if (EngineeringSensorID == nSensors)
				EngineeringSensorID = 0;
			break;
		case MENU_DOWN:
			EngineeringSensorID--;
			if (EngineeringSensorID == 255)
				EngineeringSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeEngineeringSensorID, EngineeringSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	EngineeringSensor_CallbackRender(column);
	return false;
}

# define ENGINEERING_SUBMENU_ITEMS  4
static MENU_ITEM ENGINEERING_submenu[ENGINEERING_SUBMENU_ITEMS] = {
	{"Gépház akt hõm", 		EngineeringTempActual_CallbackRender, 	EngineeringTempActual_Callback, 		0,				NULL},
	{"Gépház min hõm", 		EngineeringTempMin_CallbackRender, 		EngineeringTempMin_ActionCallback,		0,				NULL},
	{"Gépház kívánt hõm", 	EngineeringTempDesired_CallbackRender, 	EngineeringTempDesired_ActionCallback,	0,				NULL},
	{"Gépház szenzor",		EngineeringSensor_CallbackRender, 		EngineeringSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Relays, submenu definitions
*************************************************************************/

void DHWRelay_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szelep állapot"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << DHW_VALVE_RELAY))
		lcd_puts_p(PSTR("Be"));
	else
		lcd_puts_p(PSTR("Ki"));
}

bool DHWRelay_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		{
			if (Relays & DHW_VALVE_RELAY)
				Relays &= ~(1 << DHW_VALVE_RELAY);
			else
				Relays |= (1 << DHW_VALVE_RELAY);
			SwitchRelays();

			break;
		}
		case MENU_CONFIRM:
			return true;
		case MENU_CANCEL:
			return true;
	}

	DHWRelay_CallbackRender(column);
	return false;
}

void ZoneValve1_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Földszinti szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << FIRST_FLOOR_VALVE))
		lcd_puts_p(PSTR("Be"));
	else
		lcd_puts_p(PSTR("Ki"));
}

bool ZoneValve1_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relays ^= (1 << FIRST_FLOOR_VALVE);
			SwitchRelays();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ZoneValve1_CallbackRender(column);
	return false;
}

void ZoneValve2_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_p(PSTR("Emeleti szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << SECOND_FLOOR_VALVE))
			lcd_puts_p(PSTR("Be"));
		else
			lcd_puts_p(PSTR("Ki"));
}

bool ZoneValve2_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relays ^= (1 << SECOND_FLOOR_VALVE);
			SwitchRelays();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ZoneValve2_CallbackRender(column);
	return false;
}

void BufferValve_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer / gáz szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << BUFFER_VALVE_RELAY))
		lcd_puts_p(PSTR("Puffer"));
	else
		lcd_puts_hu(PSTR("Gáz"));
}

bool BufferValve_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relays ^= (1 << BUFFER_VALVE_RELAY);
			SwitchRelays();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	BufferValve_CallbackRender(column);
	return false;
}

void BufferPump_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer szivattyú"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << BUFFER_PUMP_RELAY))
		lcd_puts_p(PSTR("Be"));
	else
		lcd_puts_p(PSTR("Ki"));
}

bool BufferPump_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relays ^= (1 << BUFFER_PUMP_RELAY);
			SwitchRelays();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	BufferPump_CallbackRender(column);
	return false;
}

void GasRelay_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gáz állapot"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << GAS_RELAY))
		lcd_puts_p(PSTR("Be"));
	else
		lcd_puts_p(PSTR("Ki"));
}

bool GasRelay_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relays ^= (1 << GAS_RELAY);
			SwitchRelays();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	GasRelay_CallbackRender(column);
	return false;
}

#define RELAYS_SUBMENU_ITEMS 6
static MENU_ITEM RELAYS_submenu[RELAYS_SUBMENU_ITEMS] = {
	{"HMV 3 járatú", 			DHWRelay_CallbackRender, 			DHWRelay_ActionCallback, 		0,	NULL},
	{"Zóna földszint", 			ZoneValve1_CallbackRender, 			ZoneValve1_ActionCallback, 		0,	NULL},
	{"Zóna emelet", 			ZoneValve2_CallbackRender, 			ZoneValve2_ActionCallback, 		0,	NULL},
	{"Puffer 3 járatú", 		BufferValve_CallbackRender, 		BufferValve_ActionCallback, 	0,	NULL},
	{"Puffer szivattyú", 		BufferPump_CallbackRender, 			BufferPump_ActionCallback, 		0,	NULL},
	{"Gáz relé", 				GasRelay_CallbackRender, 			GasRelay_ActionCallback, 		0,	NULL},
};


/*************************************************************************
 Sensors, submenu definitions
*************************************************************************/

void GarageTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Garázs akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(GarageTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(GarageTempFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(GarageTempMin, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(GarageTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool GarageTemp_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	return false;
}

void GarageSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Garázs szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(GarageSensorID, buf, 10);
	lcd_puts(buf);
}

bool GarageSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			GarageSensorID++;
			if (GarageSensorID == nSensors)
				GarageSensorID = 0;
			break;
		case MENU_DOWN:
			GarageSensorID--;
			if (GarageSensorID == 255)
				GarageSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeGarageSensorID, GarageSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	GarageSensor_CallbackRender(column);
	return false;
}

void LivingRoomTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Nappali akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(LivingRoomTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(LivingRoomTempFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(LivingRoomTempMin, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(LivingRoomTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool LivingRoomTemp_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	return false;
}

void LivingRoomSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Nappali szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(LivingRoomSensorID, buf, 10);
	lcd_puts(buf);
}

bool LivingRoomSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			LivingRoomSensorID++;
			if (LivingRoomSensorID == nSensors)
				LivingRoomSensorID = 0;
			break;
		case MENU_DOWN:
			LivingRoomSensorID--;
			if (LivingRoomSensorID == 255)
				LivingRoomSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeLivingRoomSensorID, LivingRoomSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	LivingRoomSensor_CallbackRender(column);
	return false;
}

void FloorTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Padló akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(FloorTempBuf); lcd_puts_p(PSTR(".")), lcd_puts(FloorTempFracBuf); lcd_puts_p(PSTR(" C"));
	
	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(FloorTempMin, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(FloorTempMax, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool FloorTemp_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	return false;
}

void FloorSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Padló szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(FloorSensorID, buf, 10);
	lcd_puts(buf);
}

bool FloorSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			FloorSensorID++;
			if (FloorSensorID == nSensors)
				FloorSensorID = 0;
			break;
		case MENU_DOWN:
			FloorSensorID--;
			if (FloorSensorID == 255)
				FloorSensorID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeFloorSensorID, FloorSensorID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	FloorSensor_CallbackRender(column);
	return false;
}


# define SENSORS_SUBMENU_ITEMS  6
static MENU_ITEM SENSORS_submenu[SENSORS_SUBMENU_ITEMS] = {
	{"Garázs hõm",			GarageTemp_CallbackRender,			GarageTemp_Callback, 				0, NULL},
	{"Garázs szenzor ID",	GarageSensor_CallbackRender,		GarageSensor_ActionCallback,		0, NULL},
	{"Nappali hõm",			LivingRoomTemp_CallbackRender,		LivingRoomTemp_Callback,			0, NULL},
	{"Nappali szenzor ID",	LivingRoomSensor_CallbackRender,	LivingRoomSensor_ActionCallback,	0, NULL},
	{"Padló hõm",			FloorTemp_CallbackRender,			FloorTemp_Callback,					0, NULL},
	{"Padló szenzor ID",	FloorSensor_CallbackRender,			FloorSensor_ActionCallback,			0, NULL},
};


/*************************************************************************
 Menu Settings, submenu definition
*************************************************************************/
void DebugMode_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Tesztüzem"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(DebugMode, buf, 10);
	lcd_puts(buf);
}

bool DebugMode_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++DebugMode;
			break;
		case MENU_DOWN:
			--DebugMode;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDebugMode, DebugMode);
			return true;
		case MENU_CANCEL:
			return true;
	}

	DebugMode_CallbackRender(column);
	return false;
}

void MenuTimer_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Menü idõzítõ (mp)"));
	char buf[4];
	itoa(MenuTimer, buf, 10);
	lcd_puts(buf);
}

bool MenuTimer_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++MenuTimer;
			break;
		case MENU_DOWN:
			--MenuTimer;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeMenuTimer, MenuTimer);
			return true;
		case MENU_CANCEL:
			return true;
	}

	MenuTimer_CallbackRender(column);
	return false;
}

void LCDBackLight_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("LCD háttér"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(LCDBackLight, buf, 10);
	lcd_puts(buf);
}

bool LCDBackLight_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++LCDBackLight;
			break;
		case MENU_DOWN:
			--LCDBackLight;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeLCDBackLight, LCDBackLight);
			return true;
		case MENU_CANCEL:
			return true;
	}
	if (LCDBackLight != 0)
		OCR3C = LCDBackLight;

	LCDBackLight_CallbackRender(column);
	return false;
}

void PumpPlusTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Szivattyú utánfutás"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(PumpPlusTime, buf, 10);
	lcd_puts(buf);
}

bool PumpPlusTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++PumpPlusTime;
			break;
		case MENU_DOWN:
			--PumpPlusTime;
			break;
		case MENU_CONFIRM:
			eeprom_update_word(&eePumpPlusTime, PumpPlusTime);
			return true;
		case MENU_CANCEL:
			return true;
	}

	PumpPlusTime_CallbackRender(column);
	return false;
}

void ClockHour_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("óra"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(Hour, buf, 10);
	lcd_puts(buf);
}

bool ClockHour_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++Hour == 24)
				Hour = 0;
			break;
		case MENU_DOWN:
			if (--Hour == 255)
				Hour = 23;
			break;
		case MENU_CONFIRM:
			ClockInitialized = 1;
			return true;
		case MENU_CANCEL:
			return true;
	}

	ClockHour_CallbackRender(column);
	return false;
}

void ClockMinute_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_p(PSTR("perc"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(Minute, buf, 10);
	lcd_puts(buf);
}

bool ClockMinute_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++Minute == 60)
				Minute = 0;
			break;
		case MENU_DOWN:
			if (--Minute == 255)
				Minute = 59;
			break;
		case MENU_CONFIRM:
			ClockInitialized = 1;
			return true;
		case MENU_CANCEL:
			return true;
	}

	ClockMinute_CallbackRender(column);
	return false;
}

void ClockYear_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("év"));
	lcd_gotoxy(0,1);
	char buf[5];
	itoa(Year, buf, 10);
	lcd_puts(buf);
}

bool ClockYear_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			Year++;
			break;
		case MENU_DOWN:
			Year--;
			break;
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	ClockYear_CallbackRender(column);
	return false;
}

void ClockMonth_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("hónap"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(Month, buf, 10);
	lcd_puts(buf);
}

bool ClockMonth_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++Month == 13)
				Month = 1;
			break;
		case MENU_DOWN:
			if (--Month == 0)
				Month = 12;
			break;
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	ClockMonth_CallbackRender(column);
	return false;
}

void ClockDay_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("nap"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(Day, buf, 10);
	lcd_puts(buf);
}

bool ClockDay_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++Day == 32)
				Day = 1;
			break;
		case MENU_DOWN:
			if (--Day == 0)
				Day = 31;
			break;
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	ClockDay_CallbackRender(column);
	return false;
}

void ClockDayName_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("nap név"));
	lcd_gotoxy(0,1);
	switch (DayName)
	{
		case 1:
			lcd_puts_hu(PSTR("hétfõ"));
			break;
		case 2:
			lcd_puts_hu(PSTR("kedd"));
			break;
		case 3:
			lcd_puts_hu(PSTR("szerda"));
			break;
		case 4:
			lcd_puts_hu(PSTR("csütörtök"));
			break;
		case 5:
			lcd_puts_hu(PSTR("péntek"));
			break;
		case 6:
			lcd_puts_hu(PSTR("szombat"));
			break;
		case 7:
			lcd_puts_hu(PSTR("vasárnap"));
			break;
	}
}

bool ClockDayName_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++DayName == 8)
				DayName = 1;
			break;
		case MENU_DOWN:
			if (--DayName == 0)
			DayName = 7;
			break;
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}

	ClockDayName_CallbackRender(column);
	return false;
}

#define CLOCK_SUBMENU_ITEMS 6
static MENU_ITEM CLOCK_submenu[CLOCK_SUBMENU_ITEMS] = {
	{"óra", 			ClockHour_CallbackRender, 		ClockHour_ActionCallback,		0,	NULL},
	{"perc", 			ClockMinute_CallbackRender,		ClockMinute_ActionCallback,		0,	NULL},
	{"év", 				ClockYear_CallbackRender,		ClockYear_ActionCallback,		0,	NULL},
	{"hónap", 			ClockMonth_CallbackRender,		ClockMonth_ActionCallback,		0,	NULL},
	{"nap", 			ClockDay_CallbackRender,		ClockDay_ActionCallback,		0,	NULL},
	{"nap neve", 		ClockDayName_CallbackRender,	ClockDayName_ActionCallback,	0,	NULL},
};


void ComfortMode_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Komfort mód"));
	lcd_gotoxy(0,1);
	char buf[2];
	itoa(ComfortMode, buf, 10);
	lcd_puts(buf);
}

bool ComfortMode_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortMode == 2)
				ComfortMode = 0;
			break;
		case MENU_DOWN:
			if (--ComfortMode == 255)
				ComfortMode = 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortMode, ComfortMode);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ComfortMode_CallbackRender(column);
	return false;
}

void ComfortTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Komfort hõm"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(ComfortTemp, buf, 10);
	lcd_puts(buf);
}

bool ComfortTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortTemp == 35)
				ComfortTemp = 15;
			break;
		case MENU_DOWN:
			if (--ComfortTemp == 255)
				ComfortTemp = 35;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortTemp, ComfortTemp);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ComfortTemp_CallbackRender(column);
	return false;
}

void ComfortForwardTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Komfort elõremenõ"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(ComfortTemp, buf, 10);
	lcd_puts(buf);
}

bool ComfortForwardTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortForwardTemp == ForwardHeatTemp)
				ComfortForwardTemp -= 1;
			break;
		case MENU_DOWN:
			if (--ComfortForwardTemp == 255)
				ComfortForwardTemp = ForwardHeatTemp;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortForwardTemp, ComfortForwardTemp);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ComfortForwardTemp_CallbackRender(column);
	return false;
}

#define COMFORT_SUBMENU_ITEMS 3
static MENU_ITEM COMFORT_submenu[COMFORT_SUBMENU_ITEMS] = {
	{"Komfort mód", 			ComfortMode_CallbackRender, 		ComfortMode_ActionCallback,			0,	NULL},
	{"Komfort hõm", 			ComfortTemp_CallbackRender,			ComfortTemp_ActionCallback,			0,	NULL},
	{"Komfort elõremenõ", 		ComfortForwardTemp_CallbackRender,	ComfortForwardTemp_ActionCallback,	0,	NULL}
};

#define SYSPARAM_SUBMENU_ITEMS 6
static MENU_ITEM SYSPARAM_submenu[SYSPARAM_SUBMENU_ITEMS] = {
	{"Szivattyú után", 	PumpPlusTime_CallbackRender, 		PumpPlusTime_ActionCallback,	0,						NULL},
	{"Komfort mód", 	NULL,						 		NULL,							COMFORT_SUBMENU_ITEMS,	COMFORT_submenu},
	{"óra, dátum", 		NULL,				 				NULL,							CLOCK_SUBMENU_ITEMS,	CLOCK_submenu},
	{"Tesztüzem", 		DebugMode_CallbackRender, 			DebugMode_ActionCallback, 		0,						NULL},
	{"Menü idõzítõ", 	MenuTimer_CallbackRender, 			MenuTimer_ActionCallback, 		0,						NULL},
	{"LCD háttér", 		LCDBackLight_CallbackRender, 		LCDBackLight_ActionCallback,	0,						NULL},
};


/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  7
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beállítás",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     		DHW_submenu	  	},
	{"Puffer beállítás",   	NULL,                           NULL,                     BUFFER_SUBMENU_ITEMS,     	BUFFER_submenu	},
	{"Kazánház termoszt",	NULL,                           NULL,                     ENGINEERING_SUBMENU_ITEMS,	ENGINEERING_submenu	},
	{"Külsõ érzékelõ",		NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   		BME280_submenu	},
	{"Relék",				NULL,							NULL,					  RELAYS_SUBMENU_ITEMS, 		RELAYS_submenu	},
	{"Egyéb érzékelõ",		NULL,							NULL,					  SENSORS_SUBMENU_ITEMS, 		SENSORS_submenu	},
	{"Vezérlõ beállítás",	NULL,							NULL,					  SYSPARAM_SUBMENU_ITEMS, 		SYSPARAM_submenu},
};

/*
** Public methods to access home view and home view size
*/

MENU_ITEM *menuItemsGetHomeMenu(){
	return home_items;
}

uint8_t menuItemsGetHomeMenuSize(){
	return MENU_HOME_ITEMS;
}