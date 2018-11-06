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

}

bool BME280Temp_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			break;
		case MENU_DOWN:
			break;
		case MENU_CONFIRM:
			break;
		case MENU_CANCEL:
			return true;
	}

	BME280Temp_CallbackRender(column);
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
			break;
		case MENU_DOWN:
			break;
		case MENU_CONFIRM:
			break;
		case MENU_CANCEL:
			return true;
	}

	BME280Humidity_CallbackRender(column);
	return false;
}

void SwitchOnOutdoorTempMin_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Külsõ min hõm"));
	lcd_gotoxy(0,1);
	uint8_t cel = SwitchOnOutdoorTempMin % 10;
	char buf[3];
	itoa(cel, buf, 10);
	lcd_puts(buf); lcd_puts_p(PSTR("."));
	uint8_t frac = SwitchOnOutdoorTempMin % 10;
	itoa(frac, buf, 10);
	lcd_puts(buf);
}

bool SwitchOnOutdoorTempMin_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			SwitchOnOutdoorTempMin += 10;
			break;
		case MENU_DOWN:
			SwitchOnOutdoorTempMin -= 10;
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
}

bool DHWTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			break;
		case MENU_DOWN:
			break;
		case MENU_CONFIRM:
			break;
		case MENU_CANCEL:
			return true;
	}

	DHWTempActual_CallbackRender(column);
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

void DHWMinHour_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Min óra"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(DHWMinHour, buf, 10);
	lcd_puts(buf);
}

bool DHWMinHour_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++DHWMinHour == 24)
				DHWMinHour = 0;
			break;
		case MENU_DOWN:
			if (--DHWMinHour == 255)
				DHWMinHour = 23;
			break;
		case MENU_CONFIRM:
		eeprom_update_byte(&eeDHWMinHour, DHWMinHour);
		case MENU_CANCEL:
		return true;
	}

	DHWMinHour_CallbackRender(column);
	return false;
}

void DHWMinMinute_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Min Perc"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(DHWMinMinute, buf, 10);
	lcd_puts(buf);
}

bool DHWMinMinute_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		if (++DHWMinMinute == 60)
		DHWMinMinute = 0;
		break;
		case MENU_DOWN:
		if (--DHWMinMinute == 255)
		DHWMinMinute = 59;
		break;
		case MENU_CONFIRM:
		eeprom_update_byte(&eeDHWMinMinute, DHWMinMinute);
		case MENU_CANCEL:
		return true;
	}

	DHWMinMinute_CallbackRender(column);
	return false;
}

void DHWMaxHour_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Max óra"));
	lcd_gotoxy(0,1);
	char buf[3];
	itoa(DHWMaxHour, buf, 10);
	lcd_puts(buf);
}

bool DHWMaxHour_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		if (++DHWMaxHour == 24)
		DHWMaxHour = 0;
		break;
		case MENU_DOWN:
		if (--DHWMaxHour == 255)
		DHWMaxHour = 23;
		break;
		case MENU_CONFIRM:
		eeprom_update_byte(&eeDHWMaxHour, DHWMaxHour);
		case MENU_CANCEL:
		return true;
	}

	DHWMaxHour_CallbackRender(column);
	return false;
}

void DHWMaxMinute_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Max Perc"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(DHWMaxMinute, buf, 10);
	lcd_puts(buf);
}

bool DHWMaxMinute_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		if (++DHWMaxMinute == 60)
		DHWMaxMinute = 0;
		break;
		case MENU_DOWN:
		if (--DHWMaxMinute == 255)
		DHWMaxMinute = 59;
		break;
		case MENU_CONFIRM:
		eeprom_update_byte(&eeDHWMaxMinute, DHWMaxMinute);
		case MENU_CANCEL:
		return true;
	}

	DHWMaxMinute_CallbackRender(column);
	return false;
}

#define DHWHOURS_SUBMENU_ITEMS 4
static MENU_ITEM DHWHOURS_submenu[DHWHOURS_SUBMENU_ITEMS] = {
	{"Min óra", 			DHWMinHour_CallbackRender, 		DHWMinHour_ActionCallback,		0,	NULL},
	{"Min perc", 			DHWMinMinute_CallbackRender,	DHWMinMinute_ActionCallback,	0,	NULL},
	{"Max óra", 			DHWMaxHour_CallbackRender, 		DHWMaxHour_ActionCallback,		0,	NULL},
	{"Max perc", 			DHWMaxMinute_CallbackRender,	DHWMaxMinute_ActionCallback,	0,	NULL},
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
}

bool BufferTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			break;
		case MENU_DOWN:
			break;
		case MENU_CONFIRM:
			break;
		case MENU_CANCEL:
			return true;
	}

	BufferTempActual_CallbackRender(column);
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
void GarageTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(GarageTempActualBuf); lcd_puts_p(PSTR(".")), lcd_puts(GarageTempActualFracBuf); lcd_puts_p(PSTR(" C"));
}

bool GarageTempActual_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			break;
		case MENU_DOWN:
			break;
		case MENU_CONFIRM:
			break;
		case MENU_CANCEL:
			return true;
	}

	GarageTempActual_CallbackRender(column);
	return false;
}

void GarageTempMin_CallbackRender(uint8_t which){
	char buf[4];
	itoa(GarageTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház min hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool GarageTempMin_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (GarageTempMin < GarageTempDesired)
				++GarageTempMin;
			break;
		case MENU_DOWN:
			if (GarageTempMin > 5)
				--GarageTempMin;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeGarageTempMin, GarageTempMin);
			return true;
		case MENU_CANCEL:
			return true;
	}

	GarageTempMin_CallbackRender(column);
	return false;
}

void GarageTempDesired_CallbackRender(uint8_t which){
	char buf[7];
	itoa(GarageTempDesired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház kívánt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool GarageTempDesired_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (GarageTempDesired < 35)
				++GarageTempDesired;
			break;
		case MENU_DOWN:
			if (GarageTempDesired > GarageTempMin)
				--GarageTempDesired;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeGarageTempDesired, GarageTempDesired);
			return true;
		case MENU_CANCEL:
			return true;
	}
	GarageTempDesired_CallbackRender(column);
	return false;
}

void GarageSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház szenzor ID"));
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

# define GARAGE_SUBMENU_ITEMS  4
static MENU_ITEM GARAGE_submenu[GARAGE_SUBMENU_ITEMS] = {
	{"Gépház akt hõm", 		GarageTempActual_CallbackRender, 	GarageTempActual_Callback, 			0,				NULL},
	{"Gépház min hõm", 		GarageTempMin_CallbackRender, 		GarageTempMin_ActionCallback,		0,				NULL},
	{"Gépház kívánt hõm", 	GarageTempDesired_CallbackRender, 	GarageTempDesired_ActionCallback,	0,				NULL},
	{"Gépház szenzor",		GarageSensor_CallbackRender, 		GarageSensor_ActionCallback, 		0, 				NULL},
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
		case MENU_CANCEL:
			return true;
	}

	ClockMinute_CallbackRender(column);
	return false;
}

#define CLOCK_SUBMENU_ITEMS 2
static MENU_ITEM CLOCK_submenu[CLOCK_SUBMENU_ITEMS] = {
	{"óra", 			ClockHour_CallbackRender, 		ClockHour_ActionCallback,		0,	NULL},
	{"perc", 			ClockMinute_CallbackRender,		ClockMinute_ActionCallback,		0,	NULL},
};

#define SYSPARAM_SUBMENU_ITEMS 5
static MENU_ITEM SYSPARAM_submenu[SYSPARAM_SUBMENU_ITEMS] = {
	{"Szivattyú után", 	PumpPlusTime_CallbackRender, 		PumpPlusTime_ActionCallback,	0,						NULL},
	{"óra", 			NULL,				 				NULL,							CLOCK_SUBMENU_ITEMS,	CLOCK_submenu},
	{"Tesztüzem", 		DebugMode_CallbackRender, 			DebugMode_ActionCallback, 		0,						NULL},
	{"Menü idõzítõ", 	MenuTimer_CallbackRender, 			MenuTimer_ActionCallback, 		0,						NULL},
	{"LCD háttér", 		LCDBackLight_CallbackRender, 		LCDBackLight_ActionCallback,	0,						NULL},
};

/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  6
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beállítás",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     	DHW_submenu	  	},
	{"Puffer beállítás",   	NULL,                           NULL,                     BUFFER_SUBMENU_ITEMS,     BUFFER_submenu	},
	{"Kazánház termoszt",	NULL,                           NULL,                     GARAGE_SUBMENU_ITEMS,   	GARAGE_submenu	},
	{"Külsõ érzékelõ",		NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   	BME280_submenu	},
	{"Relék",				NULL,							NULL,					  RELAYS_SUBMENU_ITEMS, 	RELAYS_submenu	},
	{"Vezérlõ beállítás",	NULL,							NULL,					  SYSPARAM_SUBMENU_ITEMS, 	SYSPARAM_submenu},
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