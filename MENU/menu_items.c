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

extern uint8_t DebugMode, eeDebugMode;
extern uint8_t MenuTimer, eeMenuTimer;
extern uint8_t LCDBackLight, eeLCDBackLight;

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

extern unsigned char Pump_relays, Valve_relays, Relay_or_PWM, eeRelay_or_PWM;

extern uint8_t nSensors;

extern void SwitchPump();
extern void SwitchValve();

void ChangeMsg()
{
	lcd_clrscr();
	lcd_puts_P("Fel / le gombokkal");
	lcd_gotoxy(0,1);
	lcd_puts_hu(PSTR("lehet valtoztatni"));
	lcd_gotoxy(0,2);
	return;
}

void BME280_temp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 hom"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280_temp_buf);
}

void BME280_humidity_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 para"));
	lcd_gotoxy(0,1);
	lcd_puts(BME280_humid_buf);
}

void BME280_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(BME280_temp_min, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 min hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void BME280_desired_CallbackRender(uint8_t which){
	char buf[10];
	itoa(BME280_temp_desired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 kivant hom"));
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

	char buf[5];
	ChangeMsg();
	itoa(BME280_temp_min, buf, 10);
	lcd_puts(buf);

	return false;
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

	char buf[5];
	ChangeMsg();
	itoa(BME280_temp_desired, buf, 10);
	lcd_puts(buf);

	return false;
}

/*************************************************************************
 BME 280, submenu definition
*************************************************************************/
# define BME280_SUBMENU_ITEMS  4
static MENU_ITEM BME280_submenu[BME280_SUBMENU_ITEMS] = {
	{"BME hom",			BME280_temp_CallbackRender,			NULL, 							0, NULL},
	{"BME para",		BME280_humidity_CallbackRender,		NULL,							0, NULL},
	{"BME min hom",		BME280_min_CallbackRender, 			BME280_min_ActionCallback,  	0, NULL},
	{"BME kivant hom", 	BME280_desired_CallbackRender, 		BME280_desired_ActionCallback, 	0, NULL},
};


/*************************************************************************
 Menu DHW, submenu definition
*************************************************************************/
void DHW_temp_actual_CallbackRender(uint8_t which){
	char buf[7];
	itoa(DHW_temp_actual, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV akt hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_temp_desired_CallbackRender(uint8_t which){
	char buf[7];
	itoa(DHW_temp_desired, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV kivant hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_temp_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(DHW_temp_min, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV min hom"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_sensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szenzor ID"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(DHW_sensor_ID, buf, 10);
	lcd_puts(buf);
}

void DHW_PWM_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV pumpa tipus"));
	lcd_gotoxy(0,1);
	if (!(Relay_or_PWM & (1 << DHW)))
		lcd_puts_hu(PSTR("Rele"));
	else
		lcd_puts_P("PWM");
}

void DHW_switch_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV pumpa ßllapot"));
	lcd_gotoxy(0,1);
	if (!(Relay_or_PWM & (1 << DHW)))
	{	if (Pump_relays & (1 << DHW_RELAY))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
	}
	else
	{
		char buf[6];
		lcd_puts_P("PWM: "); itoa(DHW_PWM_OCR, buf, 10);
	}
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

	char buf[5];
	ChangeMsg();
	itoa(DHW_temp_desired, buf, 10);
	lcd_puts(buf);

	return false;
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

	char buf[5];
	ChangeMsg();
	itoa(DHW_temp_min, buf, 10);
	lcd_puts(buf);

	return false;
}

bool DHW_sensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++DHW_sensor_ID > nSensors);
				DHW_sensor_ID = 0;
			break;
		case MENU_DOWN:
			if (--DHW_sensor_ID == DS18B20_MAX_NO);
				DHW_sensor_ID = nSensors;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_sensor_ID, DHW_sensor_ID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	char buf[4];
	itoa(DHW_sensor_ID, buf, 10);
	lcd_puts(buf);

	return false;
}

bool DHW_PWM_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Relay_or_PWM ^= (1 << DHW);
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeRelay_or_PWM, Relay_or_PWM);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	if (!(Relay_or_PWM & (1 << DHW)))
		lcd_puts_hu(PSTR("Rele"));
	else
		lcd_puts_P("PWM");

	return false;
}

bool DHW_switch_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		{
			if (!(Relay_or_PWM & (1 << DHW)))
			{
				if (Pump_relays & DHW_RELAY)
					Pump_relays &= ~(1 << DHW_RELAY);
				else
					Pump_relays |= (1 << DHW_RELAY);
				SwitchPump();
			}
			else
			{
				if (DHW_PWM_OCR > 10)
					switch_on_PWM_for_DHW_pump();
				else
					switch_off_PWM_for_DHW_pump();
			}
			break;
		}
		case MENU_CONFIRM:
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	if (!(Relay_or_PWM & (1 << DHW)))
	{
		if (Pump_relays & DHW_RELAY)
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
	}
	else
	{
		char buf[6];
		lcd_puts_P("PWM: "); itoa(DHW_PWM_OCR, buf, 10);
	}

	return false;
}

#define DHW_PUMP_ITEMS 2
static MENU_ITEM DHW_pump_submenu[DHW_PUMP_ITEMS] ={
	{"HMV pumpa beall", 	DHW_PWM_CallbackRender,				DHW_PWM_ActionCallback, 			0, NULL},
	{"HMV pumpa kapcs", 	DHW_switch_CallbackRender, 			DHW_switch_ActionCallback,			0, NULL},
};

# define DHW_SUBMENU_ITEMS  5
static MENU_ITEM DHW_submenu[DHW_SUBMENU_ITEMS] = {
	{"HMV akt hom", 		DHW_temp_actual_CallbackRender, 	NULL, 								0,				NULL},
	{"HMV kivant hom",		DHW_temp_desired_CallbackRender,	DHW_temp_desired_ActionCallback,	0,				NULL},
	{"HMV min hom",			DHW_temp_min_CallbackRender,  		DHW_temp_min_ActionCallback, 		0, 				NULL},
	{"HMV szenzor",			DHW_sensor_CallbackRender, 			DHW_sensor_ActionCallback, 			0, 				NULL},
	{"HMV pumpa",			NULL,								NULL,					 			DHW_PUMP_ITEMS, DHW_pump_submenu},
};

void ZoneValve1_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Zonaszelep 1 allapot"));
	lcd_gotoxy(0,1);
	if (Valve_relays & (1 << FIRST_FLOOR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
}

void ZoneValve2_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Zonaszelep 2 allapot"));
	lcd_gotoxy(0,1);
	if (Valve_relays & (1 << SECOND_FLOOR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
}

void ThreeWayValve1_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("3 jaratu 1 allapot"));
	lcd_gotoxy(0,1);
	if (Valve_relays & (1 << SOLAR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
}

void ThreeWayValve2_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("3 jaratu 2 allapot"));
	lcd_gotoxy(0,1);
	if (Valve_relays & (1 << BUFFER_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
}

bool ZoneValve1_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Valve_relays ^= (1 << FIRST_FLOOR_VALVE);
			SwitchValve();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ChangeMsg();
	if (Valve_relays & (1 << FIRST_FLOOR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));
	return false;
}

bool ZoneValve2_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Valve_relays ^= (1 << SECOND_FLOOR_VALVE);
			SwitchValve();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ChangeMsg();
	if (Valve_relays & (1 << SECOND_FLOOR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));

	return false;
}

bool ThreeWayValve1_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Valve_relays ^= (1 << SOLAR_VALVE);
			SwitchValve();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ChangeMsg();
	if (Valve_relays & (1 << SOLAR_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));

	return false;
}

bool ThreeWayValve2_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			Valve_relays ^= (1 << BUFFER_VALVE);
			SwitchValve();
			break;
		case MENU_CONFIRM:
		return true;
		case MENU_CANCEL:
		return true;
	}

	ChangeMsg();
	if (Valve_relays & (1 << BUFFER_VALVE))
			lcd_puts_hu(PSTR("Rele bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Rele kikapcsolva"));

	return false;
}


#define VALVES_SUBMENU_ITEMS 4
static MENU_ITEM VALVES_submenu[VALVES_SUBMENU_ITEMS] = {
	{"Z¡na 1", 			ZoneValve1_CallbackRender, 			ZoneValve1_ActionCallback, 		0,	NULL},
	{"Z¡na 2", 			ZoneValve2_CallbackRender, 			ZoneValve2_ActionCallback, 		0,	NULL},
	{"3 jßratÿ 1", 		ThreeWayValve1_CallbackRender, 		ThreeWayValve1_ActionCallback, 	0,	NULL},
	{"3 jßratÿ 2", 		ThreeWayValve2_CallbackRender, 		ThreeWayValve2_ActionCallback, 	0,	NULL},
};


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

	ChangeMsg();
	char buf[4];
	itoa(DebugMode, buf, 10);
	lcd_puts(buf);

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

	ChangeMsg();
	char buf[4];
	itoa(MenuTimer, buf, 10);
	lcd_puts(buf);

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
		OCR3A = LCDBackLight;

	ChangeMsg();
	char buf[4];
	itoa(LCDBackLight, buf, 10);
	lcd_puts(buf);

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
#define MENU_HOME_ITEMS  4
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beallitas",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     	DHW_submenu	  	},
	{"Kazanhaz termosztat",	NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   	BME280_submenu	},
	{"Szelepek",			NULL,							NULL,					  VALVES_SUBMENU_ITEMS, 	VALVES_submenu	},
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