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
void BME280_temp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 hom"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280_temp_buf);
}

bool BME280_temp_Callback(MENU_BUTTON *button, uint8_t column){
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

	BME280_temp_CallbackRender(column);
	return false;
}

void BME280_humidity_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 para"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280_humid_buf);
}

bool BME280_humidity_Callback(MENU_BUTTON *button, uint8_t column){
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

	BME280_humidity_CallbackRender(column);
	return false;
}

void BME280_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(BME280_temp_min, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 min hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool BME280_min_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++BME280_temp_min;
			break;
		case MENU_DOWN:
			--BME280_temp_min;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeBME280_temp_min, BME280_temp_min);
			return true;
		case MENU_CANCEL:
			return true;
	}

	BME280_min_CallbackRender(column);
	return false;
}

void BME280_desired_CallbackRender(uint8_t which){
	char buf[10];
	itoa(BME280_temp_desired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 kivant hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool BME280_desired_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++BME280_temp_desired;
			break;
		case MENU_DOWN:
			--BME280_temp_desired;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeBME280_temp_desired, BME280_temp_desired);
			return true;
		case MENU_CANCEL:
			return true;
	}

	BME280_desired_CallbackRender(column);
	return false;
}


# define BME280_SUBMENU_ITEMS  4
static MENU_ITEM BME280_submenu[BME280_SUBMENU_ITEMS] = {
	{"BME hom",			BME280_temp_CallbackRender,			BME280_temp_Callback, 	0, NULL},
	{"BME para",		BME280_humidity_CallbackRender,		BME280_humidity_Callback,	0, NULL},
	{"BME min hom",		BME280_min_CallbackRender, 			BME280_min_ActionCallback,  	0, NULL},
	{"BME kivant hom", 	BME280_desired_CallbackRender, 		BME280_desired_ActionCallback, 	0, NULL},
};


/*************************************************************************
 Menu DHW, submenu definitions
*************************************************************************/
void DHW_temp_actual_CallbackRender(uint8_t which){
	char buf[7];
	itoa(DHW_temp_actual, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool DHW_temp_actual_Callback(MENU_BUTTON *button, uint8_t column){
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

	DHW_temp_actual_CallbackRender(column);
	return false;
}

void DHW_temp_desired_CallbackRender(uint8_t which){
	char buf[7];
	itoa(DHW_temp_desired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV kivant hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool DHW_temp_desired_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++DHW_temp_desired;
			break;
		case MENU_DOWN:
			--DHW_temp_desired;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_temp_desired, DHW_temp_desired);
			return true;
		case MENU_CANCEL:
			return true;
	}
	DHW_temp_desired_CallbackRender(column);
	return false;
}

void DHW_temp_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(DHW_temp_min, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV min hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool DHW_temp_min_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++DHW_temp_min;
			break;
		case MENU_DOWN:
			--DHW_temp_min;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_temp_min, DHW_temp_min);
			return true;
		case MENU_CANCEL:
			return true;
	}

	DHW_temp_min_CallbackRender(column);
	return false;
}

void DHW_sensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(DHW_sensor_ID, buf, 10);
	lcd_puts(buf);
}

bool DHW_sensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			DHW_sensor_ID++;
			if (DHW_sensor_ID == nSensors)
				DHW_sensor_ID = 0;
			break;
		case MENU_DOWN:
			DHW_sensor_ID--;
			if (DHW_sensor_ID == 255)
				DHW_sensor_ID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_sensor_ID, DHW_sensor_ID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	DHW_sensor_CallbackRender(column);
	return false;
}

# define DHW_SUBMENU_ITEMS  4
static MENU_ITEM DHW_submenu[DHW_SUBMENU_ITEMS] = {
	{"HMV akt hom", 		DHW_temp_actual_CallbackRender, 	DHW_temp_actual_Callback, 			0,				NULL},
	{"HMV kivant hom",		DHW_temp_desired_CallbackRender,	DHW_temp_desired_ActionCallback,	0,				NULL},
	{"HMV min hom",			DHW_temp_min_CallbackRender,  		DHW_temp_min_ActionCallback, 		0, 				NULL},
	{"HMV szenzor",			DHW_sensor_CallbackRender, 			DHW_sensor_ActionCallback, 			0, 				NULL},
};

/*************************************************************************
 Menu Buffer, submenu definitions
*************************************************************************/
void BufferTempActual_CallbackRender(uint8_t which){
	char buf[7];
	itoa(Buffer_temp_actual, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
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
	itoa(Forward_heat_temp, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Eloremeno hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool ForwardHeatTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++Forward_heat_temp;
			break;
		case MENU_DOWN:
			--Forward_heat_temp;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeForward_heat_temp, Forward_heat_temp);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ForwardHeatTemp_CallbackRender(column);
	return false;
}

void Buffer_sensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(Buffer_sensor_ID, buf, 10);
	lcd_puts(buf);
}

bool Buffer_sensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			Buffer_sensor_ID++;
			if (Buffer_sensor_ID == nSensors)
				Buffer_sensor_ID = 0;
			break;
		case MENU_DOWN:
			Buffer_sensor_ID--;
			if (Buffer_sensor_ID == 255)
				Buffer_sensor_ID = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeBuffer_sensor_ID, Buffer_sensor_ID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	Buffer_sensor_CallbackRender(column);
	return false;
}

# define BUFFER_SUBMENU_ITEMS  3
static MENU_ITEM BUFFER_submenu[BUFFER_SUBMENU_ITEMS] = {
	{"Buffer akt hom", 		BufferTempActual_CallbackRender, 	BufferTempActual_Callback, 			0,				NULL},
	{"Eloremeno hom", 		ForwardHeatTemp_CallbackRender, 	ForwardHeatTemp_ActionCallback,		0,				NULL},
	{"Buffer szenzor",		Buffer_sensor_CallbackRender, 		Buffer_sensor_ActionCallback, 		0, 				NULL},
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
	if (DebugMode != 0)
			lcd_puts_hu(PSTR("Bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Kikapcsolva"));
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


#define SYSPARAM_SUBMENU_ITEMS 3
static MENU_ITEM SYSPARAM_submenu[SYSPARAM_SUBMENU_ITEMS] = {
	{"Tesztuzem", 		DebugMode_CallbackRender, 			DebugMode_ActionCallback, 		0,	NULL},
	{"Menu idozito", 	MenuTimer_CallbackRender, 			MenuTimer_ActionCallback, 		0,	NULL},
	{"LCD hatter", 		LCDBackLight_CallbackRender, 		LCDBackLight_ActionCallback,	0,	NULL},

};

/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  5
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beallitas",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     	DHW_submenu	  	},
	{"Puffer beallitas",   	NULL,                           NULL,                     BUFFER_SUBMENU_ITEMS,     BUFFER_submenu	},
	{"Kazanhaz termoszt",	NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   	BME280_submenu	},
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