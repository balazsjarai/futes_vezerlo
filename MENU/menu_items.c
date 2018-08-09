#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "../LCD/lcd.h"
#include "menu.h"
#include "menu_items.h"
#include "../Utils/ftoa.h"

extern volatile float BME280_temp, BME280_humid;
extern volatile int BME280_temp_min, BME280_temp_desired;

extern volatile unsigned int eeBME280_temp_min, eeBME280_temp_desired;

extern volatile uint8_t menutimer;

extern volatile uint16_t DHW_temp_actual, DHW_temp_desired, DHW_temp_min;
extern volatile uint16_t eeDHW_temp_actual, eeDHW_temp_desired, eeDHW_temp_min;

extern volatile unsigned char DHW_sensor_ID;
extern volatile unsigned char eeDHW_sensor_ID;

extern volatile uint16_t solar_temp_actual, solar_temp_desired, solar_temp_up_threshold;
extern volatile uint16_t buffer_temp_actual;

extern volatile char DHW_PWM, solar_pump, buffer_pump;

extern unsigned char relays_1;

/*
** Callback functions menu
*/


void BME280_temp_CallbackRender(uint8_t which){
	char buf[7];
	ftoa(buf, bme280_temp, 2);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 hõm")9;
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void BME280_humidity_CallbackRender(uint8_t which){
	char buf[5];
	ftoa(buf, bme280_humid, 2);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 pára"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void BME280_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(buf, bme280_min, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 min hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void BME280_desired_CallbackRender(uint8_t which){
	char buf[10];
	itoa(buf, bme280_min, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("BME280 kívant hõm"));
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
	{"BME hõm",			BME280_temp_CallbackRender,			NULL, 							0, NULL},
	{"BME pára",		BME280_humidity_CallbackRender,		NULL,							0, NULL},
	{"BME min hõm",		BME280_min_CallbackRender, 			BME280_min_ActionCallback,  	0, NULL},
	{"BME kívánt hõm", 	BME280_desired_CallbackRender, 		BME280_desired_ActionCallback, 	0, NULL},
};


/*************************************************************************
 Menu DHW, submenu definition
*************************************************************************/
void DHW_temp_actual_CallbackRender(uint8_t which){
	char buf[7];
	itoa(buf, DHW_temp_actual, 10);
	lcd_clrscr();
	lcd_puts_hu((PSTR("HMV akt hõm");
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_temp_desired_CallbackRender(uint8_t which){
	char buf[7];
	itoa(buf, DHW_temp_desired, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV kívánt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_temp_min_CallbackRender(uint8_t which){
	char buf[10];
	itoa(buf, DHW_temp_min, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV min hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

void DHW_sensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV szenzor ID"));
	lcd_gotoxy(0,1);
	lcd_puts(DHW_sensor_ID);
}

void DHW_PWM_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV pumpa típus"));
	lcd_gotoxy(0,1);
	if (DHW_PWM == 0)
		lcd_puts_hu(PSTR("Relé"));
	else
		lcd_puts_P("PWM");
}

void DHW_switch_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV pumpa állapot"));
	lcd_gotoxy(0,1);
	if (DHW_PWM == 0)
	{	if (relays_1 & DHW_RELAY)
			lcd_puts_hu(PSTR("Relé bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Relé kikapcsolva"));
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
			if (--DHW_sensor_ID == 255);
				DHW_sensor_ID = nSensors;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_sensor_ID, DHW_sensor_ID);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	lcd_puts(DHW_sensor_ID);

	return false;
}

bool DHW_PWM_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			DHW_PWM ^= (1 << DHW_PWM);
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHW_PWM, DHW_PWM);
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	if (DHW_PWM == 0)
		lcd_puts_hu(PSTR("RELé"));
	else
		lcd_puts_P("PWM");

	return false;
}

bool DHW_switch_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			if (DHW_PWM == 0)
			{
				if (relays_1 & DHW_RELAY)
					switch_off_DHW_relay;
				else
					switch_on_DHW_relay;
			}
			else
			{
				if (DHW_PWM_OCR < 10)
					switch_on_PWM_for_DHW_pump();
				else
					switch_off_PWM_for_DHW_pump();
			}
			break;
		case MENU_CONFIRM:
			return true;
		case MENU_CANCEL:
			return true;
	}

	ChangeMsg();
	if (DHW_PWM == 0)
	{	if (relays_1 & DHW_RELAY)
			lcd_puts_hu(PSTR("Relé bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Relé kikapcsolva"));
	}
	else
	{
		char buf[6];
		lcd_puts_P("PWM: "); itoa(DHW_PWM_OCR, buf, 10);
	}

	return false;
}

# define DHW_SUBMENU_ITEMS  5
static MENU_ITEM DHW_submenu[DHW_SUBMENU_ITEMS] = {
	{"HMV akt hõm", 		DHW_temp_actual_CallbackRender, 	menuUniversalCallback, 				0, NULL},
	{"HMV kívánt hõm",		DHW_temp_desired_CallbackRender,	DHW_temp_desired_ActionCallback,	0, NULL},
	{"HMV min hõm",			DHW_temp_min_CallbackRender,  		DHW_temp_min_ActionCallback, 		0, NULL},
	{"HMV szenzor",			DHW_sensor_CallbackRender, 			DHW_sensor_ActionCallback, 			0, NULL},
	{"HMV pumpa",			NULL,								NULL,					 			DHW_PUMP_ITEMS, DHW_pump_submenu},
};

#define DHW_PUMP_ITEMS 2
static MENU_ITEM DHW_pump_submenu[DHW_PUMP_ITEMS] ={
	{"HMV pumpa beáll", 	DHW_PWM_CallbackRender,				DHW_PWM_ActionCallback, 			0, NULL},
	{"HMV pumpa kapcs", 	DHW_switch_CallbackRender, 			DHW_switch_ActionCallback			0, NULL},
}

#define VALVES_SUBMENU_ITEMS 3
static MENU_ITEM VALVES_submenu[VALVES_SUBMENU_ITEMS] = {
	{"Zóna 1", 			ZoneValve1_CallbackRender, 		ZoneValve1_ActionCallback, 		0,	NULL},
	{"Zóna 2", 			ZoneValve2_CallbackRender, 		ZoneValve2_ActionCallback, 		0,	NULL},
	{"3 járatú 1", 		ThreeWayValve1_CallbackRender, 	ThreeWayValve1_ActionCallback, 	0,	NULL},
	{"3 járatú 2", 		ThreeWayValve1_CallbackRender, 	ThreeWayValve1_ActionCallback, 	0,	NULL},
}

void ZoneValve1_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV pumpa állapot"));
	lcd_gotoxy(0,1);
	if (DHW_PWM == 0)
	{	if (relays_1 & DHW_RELAY)
			lcd_puts_hu(PSTR("Relé bekapcsolva"));
		else
			lcd_puts_hu(PSTR("Relé kikapcsolva"));
	}
	else
	{
		char buf[6];
		lcd_puts_P("PWM: "); itoa(DHW_PWM_OCR, buf, 10);
	}
}

/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  2
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beállítás",   NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     	DHW_submenu	  	},
	{"BME280",	   		NULL,                           NULL,                     BME280_SUBMENU_ITEMS,   	BME280_submenu	},
	{"Szelepek",		NULL,							NULL,					  VALVES_SUBMENU_ITEMS, 	VALVES_submenu	},
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

void ChangeMsg()
{
	lcd_clrscr();
	lcd_puts_P("Fel / le gombokkal");
	lcd_gotoxy(0,1);
	lcd_puts_hu(PSTR("lehet változtatni"));
	lcd_gotoxy(0,2);
	return;
}