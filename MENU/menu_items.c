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
	lcd_puts_hu(PSTR("BME280 hom"));
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
	lcd_puts_hu(PSTR("BME280 para"));
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
	char buf[10];
	itoa(SwitchOnOutdoorTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Kulso min hom"));
	lcd_gotoxy(0,1);
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
	{"Kulso hom",			BME280Temp_CallbackRender,				BME280Temp_Callback, 				0, NULL},
	{"Kulso para",			BME280Humidity_CallbackRender,			BME280Humidity_Callback,			0, NULL},
	{"Kulso hom tiltas",	SwitchOnOutdoorTempMin_CallbackRender,	SwitchOnOutdoorTempMin_ActionCallback,	0, NULL},
};


/*************************************************************************
 Menu DHW, submenu definitions
*************************************************************************/
void DHWTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(DHWTempActualBuf); lcd_puts("."), lcd_puts(DHWTempActualFracBuf); lcd_puts(" C");
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
	char buf[7];
	itoa(DHWTempDesired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV kivant hom"));
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
	char buf[10];
	itoa(DHWTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV min hom"));
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

# define DHW_SUBMENU_ITEMS  4
static MENU_ITEM DHW_submenu[DHW_SUBMENU_ITEMS] = {
	{"HMV akt hom", 		DHWTempActual_CallbackRender, 	DHWTempActual_Callback, 		0,				NULL},
	{"HMV kivant hom",		DHWTempDesired_CallbackRender,	DHWTempDesired_ActionCallback,	0,				NULL},
	{"HMV min hom",			DHWTempMin_CallbackRender,  	DHWTempMin_ActionCallback, 		0, 				NULL},
	{"HMV szenzor",			DHWSensor_CallbackRender, 		DHWSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Buffer, submenu definitions
*************************************************************************/
void BufferTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(BufferTempActualBuf); lcd_puts("."), lcd_puts(BufferTempActualFracBuf); lcd_puts(" C");
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
	char buf[10];
	itoa(ForwardHeatTemp, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Eloremeno hom"));
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
	{"Buffer akt hom", 		BufferTempActual_CallbackRender, 	BufferTempActual_Callback, 			0,				NULL},
	{"Eloremeno hom", 		ForwardHeatTemp_CallbackRender, 	ForwardHeatTemp_ActionCallback,		0,				NULL},
	{"Buffer szenzor",		BufferSensor_CallbackRender, 		BufferSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Garage, submenu definitions
*************************************************************************/
void GarageTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gephaz akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(GarageTempActualBuf); lcd_puts("."), lcd_puts(GarageTempActualFracBuf); lcd_puts(" C");
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
	char buf[10];
	itoa(GarageTempMin, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gephaz min hom"));
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
	lcd_puts_hu(PSTR("Gephaz kivant hom"));
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
	lcd_puts_hu(PSTR("Gephaz szenzor ID"));
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
	{"Gephaz akt hom", 		GarageTempActual_CallbackRender, 	GarageTempActual_Callback, 			0,				NULL},
	{"Gephaz min hom", 		GarageTempMin_CallbackRender, 		GarageTempMin_ActionCallback,		0,				NULL},
	{"Gephaz kivant hom", 	GarageTempDesired_CallbackRender, 	GarageTempDesired_ActionCallback,	0,				NULL},
	{"Gephaz szenzor",		GarageSensor_CallbackRender, 		GarageSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Relays, submenu definitions
*************************************************************************/

void DHWRelay_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szelep allapot"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << DHW_VALVE_RELAY))
		lcd_puts_hu(PSTR("Be"));
	else
		lcd_puts_hu(PSTR("Ki"));
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
	lcd_puts_hu(PSTR("Foldszinti szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << FIRST_FLOOR_VALVE))
		lcd_puts_hu(PSTR("Be"));
	else
		lcd_puts_hu(PSTR("Ki"));
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
	lcd_puts_hu(PSTR("Emeleti szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << SECOND_FLOOR_VALVE))
			lcd_puts_hu(PSTR("Be"));
		else
			lcd_puts_hu(PSTR("Ki"));
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
	lcd_puts_hu(PSTR("Puffer / gaz szelep"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << BUFFER_VALVE_RELAY))
		lcd_puts_hu(PSTR("Puffer"));
	else
		lcd_puts_hu(PSTR("Gaz"));
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
	lcd_puts_hu(PSTR("Puffer szivattyu"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << BUFFER_PUMP_RELAY))
		lcd_puts_hu(PSTR("Be"));
	else
		lcd_puts_hu(PSTR("Ki"));
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
	lcd_puts_hu(PSTR("Gaz allapot"));
	lcd_gotoxy(0,1);
	if (Relays & (1 << GAS_RELAY))
		lcd_puts_hu(PSTR("Be"));
	else
		lcd_puts_hu(PSTR("Ki"));
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
	{"HMV 3 jaratu", 			DHWRelay_CallbackRender, 			DHWRelay_ActionCallback, 		0,	NULL},
	{"Zona foldszint", 			ZoneValve1_CallbackRender, 			ZoneValve1_ActionCallback, 		0,	NULL},
	{"Zona emelet", 			ZoneValve2_CallbackRender, 			ZoneValve2_ActionCallback, 		0,	NULL},
	{"Puffer 3 jaratu", 		BufferValve_CallbackRender, 		BufferValve_ActionCallback, 	0,	NULL},
	{"Puffer szivattyu", 		BufferPump_CallbackRender, 			BufferPump_ActionCallback, 		0,	NULL},
	{"Gaz rele", 				GasRelay_CallbackRender, 			GasRelay_ActionCallback, 		0,	NULL},
};

/*************************************************************************
 Menu Settings, submenu definition
*************************************************************************/
void DebugMode_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Tesztuzem"));
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
	lcd_puts_hu(PSTR("Menu idozito (mp)"));
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
	lcd_puts_hu(PSTR("LCD hatter"));
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
	lcd_puts_hu(PSTR("Szivattyu utanfutas"));
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


#define SYSPARAM_SUBMENU_ITEMS 4
static MENU_ITEM SYSPARAM_submenu[SYSPARAM_SUBMENU_ITEMS] = {
	{"Szivattyu utan", 	PumpPlusTime_CallbackRender, 		PumpPlusTime_ActionCallback,	0,	NULL},
	{"Tesztuzem", 		DebugMode_CallbackRender, 			DebugMode_ActionCallback, 		0,	NULL},
	{"Menu idozito", 	MenuTimer_CallbackRender, 			MenuTimer_ActionCallback, 		0,	NULL},
	{"LCD hatter", 		LCDBackLight_CallbackRender, 		LCDBackLight_ActionCallback,	0,	NULL},
};

/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  6
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beallitas",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     	DHW_submenu	  	},
	{"Puffer beallitas",   	NULL,                           NULL,                     BUFFER_SUBMENU_ITEMS,     BUFFER_submenu	},
	{"Kazanhaz termoszt",	NULL,                           NULL,                     GARAGE_SUBMENU_ITEMS,   	GARAGE_submenu	},
	{"Kulso erzekelo",		NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   	BME280_submenu	},
	{"Relek",				NULL,							NULL,					  RELAYS_SUBMENU_ITEMS, 	RELAYS_submenu	},
	{"Vezerlo beallitas",	NULL,							NULL,					  SYSPARAM_SUBMENU_ITEMS, 	SYSPARAM_submenu},
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