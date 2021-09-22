#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "../LCD/lcd.h"
#include "menu.h"
#include "menu_items.h"
#include "../Utils/ftoa.h"
#include "../main.h"
#include "avr/eeprom.h"
#include "../main.h"

bool Simple_Callback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
		case MENU_CANCEL:
			return true;
	}
	return false;
}

uint8_t FindActSensor(uint8_t id[])
{
	for (uint8_t i = 0; i < nSensors; i++)
	{
		if (memcmp(gSensorIDs[i], id, sizeof(uint8_t)*OW_ROMCODE_SIZE) == 0)
			return i;
	}
	return 0;
}

/*************************************************************************
 BME 280, submenu definitions
*************************************************************************/
void HIHxxxxTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HIHxxxx hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(HIHxxxxTempBuf);

	//char buffer[7];
	//lcd_gotoxy(0,2);
	//ftoa(buffer, HIHxxxxTempMin, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	//ftoa(buffer, HIHxxxxTempMax, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

void HIHxxxxHumidity_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HIHxxxx pára"));
	lcd_gotoxy(0,1);
	lcd_puts(HIHxxxxHumidBuf);
}

void SwitchOnOutdoorTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Külsõ hõm tiltás"));
	lcd_gotoxy(0,1);
	if (SwitchOnOutdoorTemp)
		lcd_puts_hu(PSTR("aktív"));
	else
		lcd_puts_hu(PSTR("letiltva"));
}

bool SwitchOnOutdoorTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
			SwitchOnOutdoorTemp ^= (1 << 0);			
		break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeSwitchOnOutdoorTemp, SwitchOnOutdoorTemp);
		return true;
		case MENU_CANCEL:
		return true;
	}

	SwitchOnOutdoorTemp_CallbackRender(column);
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
			if (SwitchOnOutdoorTempMin > 349)
				SwitchOnOutdoorTempMin = 31;
			else
				SwitchOnOutdoorTempMin++;
			break;
		case MENU_DOWN:
			if (SwitchOnOutdoorTempMin > 31)
				SwitchOnOutdoorTempMin--;
			else
				SwitchOnOutdoorTempMin = 350;
			break;
		case MENU_CONFIRM:
			eeprom_update_word(&eeSwitchOnOutdoorTempMin, (uint16_t)SwitchOnOutdoorTempMin);
			return true;
		case MENU_CANCEL:
			return true;
	}

	SwitchOnOutdoorTempMin_CallbackRender(column);
	return false;
}

# define HIHxxxx_SUBMENU_ITEMS  4
static MENU_ITEM HIHxxxx_submenu[HIHxxxx_SUBMENU_ITEMS] = {
	{"Külsõ hõm",			HIHxxxxTemp_CallbackRender,				Simple_Callback, 						0, NULL},
	{"Külsõ pára",			HIHxxxxHumidity_CallbackRender,			Simple_Callback,						0, NULL},
	{"Külsõ hõm tiltás",	SwitchOnOutdoorTemp_CallbackRender,		SwitchOnOutdoorTemp_ActionCallback,		0, NULL},
	{"Külsõ hõm tiltás C",	SwitchOnOutdoorTempMin_CallbackRender,	SwitchOnOutdoorTempMin_ActionCallback,	0, NULL},
};

/*************************************************************************
 Menu Wood, submenu definitions
*************************************************************************/
void Wood_CallbackRender(uint8_t which)
{
	lcd_clrscr();
	lcd_puts_hu(PSTR("Vegyes kazán "));
	lcd_gotoxy(13,0);
	switch (which)
	{
		case 0:
			lcd_puts_hu(PSTR("hõm"));
			lcd_gotoxy(0,1);
			lcd_puts(Wood.ActualTempBuffer); lcd_puts_p(PSTR(".")); lcd_puts(Wood.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
		break;
		
		case 1:
			lcd_puts_hu(PSTR("nyomás"));
			
			lcd_gotoxy(0,1);
			lcd_puts_hu(PSTR("elõtt"));
			lcd_gotoxy(10,1);
			lcd_puti16(WoodPressureBefore);
			
			lcd_gotoxy(0,2);
			lcd_puts_hu(PSTR("után"));
			lcd_gotoxy(10,2);
			lcd_puti16(WoodPressureAfter);
			
		break;
		
		case 2:
			lcd_puts_hu(PSTR("sziv dt"));
			lcd_gotoxy(0,1);
			lcd_puti(WoodDelta); lcd_puts_p(PSTR(" C"));
		break;
		
		case 3:
			lcd_puts_hu(PSTR("nyom r"));
			lcd_gotoxy(0,1);
			lcd_puti16(WoodPressureAlert);
		break;
		
		case 4:
			lcd_puts_hu(PSTR("rárak"));
			lcd_gotoxy(0,1);
			lcd_puti(WoodToPut);
		break;
		
		case 5:
			lcd_puts_hu(PSTR("hõm r."));
			lcd_gotoxy(0,1);
			lcd_puti(WoodTempAlert);		
		break;
		
		case 6:
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("ID"));
			lcd_gotoxy(0,1);
			for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
			{
				lcd_putbyte_hex(Wood.SensorID[i]);
			}
		break;
	}
	
}

bool Wood_ActionCallback(MENU_BUTTON *button, uint8_t column)
{
	uint8_t actSensor = FindActSensor(Wood.SensorID);
	switch(button->role){
		case MENU_UP:
		switch (column)
		{
			case 2:
				++WoodDelta;
			break;
			
			case 3:
				++WoodPressureAlert;
			break;
			
			case 4:
				++WoodToPut;
			break;
			
			case 5:
				++WoodTempAlert;
			break;
			
			case 6:
				if (++actSensor == nSensors)
					actSensor = 0;
			break;
		}
		break;
		
		case MENU_DOWN:
		switch (column)
		{		
			case 2:
				--WoodDelta;
			break;
				
			case 3:
				--WoodPressureAlert;
			break;
			
			case 4:
				--WoodToPut;
			break;
			
			case 5:
				--WoodTempAlert;
			break;
			
			case 6:
				if (--actSensor == 255)
					actSensor = nSensors - 1;
			break;
		}
		
		
		break;
		
		case MENU_CONFIRM:
		switch (column)
		{
			case 0:
			case 1:
				return true;
				
			case 2:
				eeprom_update_byte(&eeWoodDelta, WoodDelta);
			break;
				
			case 3:
				eeprom_update_word(&eeWoodPressureAlert, WoodPressureAlert);
			break;
			
			case 4:
				eeprom_update_byte(&eeWoodToPut, WoodToPut);
			break;
			
			case 5:
				eeprom_update_byte(&eeWoodTempAlert, WoodTempAlert);
			break;
			
			case 6:
				eeprom_update_block(gSensorIDs[actSensor], &eeWoodSensorID, OW_ROMCODE_SIZE);
			break;
		}
		return true;
		
		case MENU_CANCEL:
			return true;
	}
	memcpy(&Wood.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	Wood_CallbackRender(column);
	return false;
}

# define Wood_SUBMENU_ITEMS  7
static MENU_ITEM Wood_submenu[Wood_SUBMENU_ITEMS] = {
	{"Vegyes hõm",				Wood_CallbackRender,			Wood_ActionCallback, 						0, NULL},
	{"Vegyes nyomás",			Wood_CallbackRender,			Wood_ActionCallback, 						0, NULL},
	{"Vegyes sziv kapcs.",		Wood_CallbackRender,			Wood_ActionCallback, 						0, NULL},
	{"Vegyes ny. riaszt",		Wood_CallbackRender,			Wood_ActionCallback,						0, NULL},
	{"Vegyes h. leég",			Wood_CallbackRender,			Wood_ActionCallback,						0, NULL},	
	{"Vegyes h. riaszt",		Wood_CallbackRender,			Wood_ActionCallback,						0, NULL},	
	{"Vegyes szenzor ID",		Wood_CallbackRender,			Wood_ActionCallback,						0, NULL},
};

/*************************************************************************
 Menu DHW, submenu definitions
*************************************************************************/
void DHWTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("HMV akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(DHW.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(DHW.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(DHW.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(DHW.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
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
	//char buf[4];
	//itoa(DHW.SensorID, buf, 10);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(DHW.SensorID[i]);
	}
	
}

bool DHWSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(DHW.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:			
			eeprom_update_block(gSensorIDs[actSensor], &eeDHWSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&DHW.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	DHWSensor_CallbackRender(column);
	return false;
}

void DHWMinTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Min idõ"));
	lcd_gotoxy(0,1);
	lcd_puti(DHWMinTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(DHWMinTimeMinute);
	//char buf[5];
	//itoa(DHWMinTime, buf, 10);
	//lcd_puts(buf);
}

bool DHWMinTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++DHWMinTimeMinute == 60)
			{
				DHWMinTimeMinute = 0;
				if (++DHWMinTimeHour == 24)
				DHWMinTimeHour = 0;
			}
		break;
		case MENU_DOWN:
			if (--DHWMinTimeMinute == 255)
			{
				DHWMinTimeMinute = 59;
				if (--DHWMinTimeHour == 255)
					DHWMinTimeHour = 23;
			}
			//else
			//	DHWMaxTime -= 10;
		break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHWMinTimeHour, DHWMinTimeHour);
			eeprom_update_byte(&eeDHWMinTimeMinute, DHWMinTimeMinute);
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
	lcd_puti(DHWMaxTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(DHWMaxTimeMinute);
	//char buf[5];
	//itoa(DHWMaxTimeHour, buf, 10);
	//lcd_puts(buf);
}

bool DHWMaxTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++DHWMaxTimeMinute == 60)
			{
				DHWMaxTimeMinute = 0;
				if (++DHWMaxTimeHour == 24)
					DHWMaxTimeHour = 0;
			}
			break;
		case MENU_DOWN:
			if (--DHWMaxTimeMinute == 255)
			{
				DHWMaxTimeMinute = 59;
				if (--DHWMaxTimeHour == 255)
					DHWMaxTimeHour = 23;
			}
			//else
			//	DHWMaxTime -= 10;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeDHWMaxTimeHour, DHWMaxTimeHour);
			eeprom_update_byte(&eeDHWMaxTimeMinute, DHWMaxTimeMinute);
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
	{"HMV akt hõm", 		DHWTempActual_CallbackRender, 	Simple_Callback, 				0,						NULL},
	{"HMV kívánt hõm",		DHWTempDesired_CallbackRender,	DHWTempDesired_ActionCallback,	0,						NULL},
	{"HMV min hõm",			DHWTempMin_CallbackRender,  	DHWTempMin_ActionCallback, 		0, 						NULL},
	{"HMV órák",			NULL,							NULL,							DHWHOURS_SUBMENU_ITEMS,	DHWHOURS_submenu},
	{"HMV szenzor",			DHWSensor_CallbackRender, 		DHWSensor_ActionCallback, 		0, 						NULL},
};

/*************************************************************************
 Menu SecondFloor, submenu definitions
*************************************************************************/
void SecondFloor_CallbackRender(uint8_t which)
{
	lcd_clrscr();
	lcd_puts_hu(PSTR("Felsõ szint"));
	char buf1[5];
	char buf2[2];
	switch (which)
	{
		case 0:
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("hõm"));
			lcd_gotoxy(0,1);
			lcd_puts(SecondFloor.ActualTempBuffer); lcd_puts_p(PSTR(".")); lcd_puts(SecondFloor.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
		break;
		
		case 1:			
			itoa(SecondFloorDesired, buf1, 10);
			strcat(buf1, ".");
			itoa(SecondFloorFracDesired, buf2, 10);
			strcat(buf1, buf2);			
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("kívánt"));
			lcd_gotoxy(0,1);
			lcd_puts(buf1);
		break;
		
		case 2:
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("term."));
			lcd_gotoxy(0,1);
			if (SecondFloorThermostatActive)
				lcd_puts_p(PSTR("bekapcsolva")); 
			else
				lcd_puts_p(PSTR("kikapcsolva"));
		break;
		
		case 3:
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("hiszt."));
			lcd_gotoxy(0,1);
			lcd_puts_p(PSTR("0.")); lcd_putc((char)(SecondFloorHysteresis + 48)); lcd_puts_p(PSTR(" C"));
		break;
		
		case 4:
			lcd_clrscr();
			lcd_puts_hu(PSTR("Min idõ"));
			lcd_gotoxy(0,1);
			lcd_puti(SecondFloorMinTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(SecondFloorMinTimeMinute);
		break;
		
		case 5:
			lcd_clrscr();
			lcd_puts_hu(PSTR("Max idõ"));
			lcd_gotoxy(0,1);
			lcd_puti(SecondFloorMaxTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(SecondFloorMaxTimeMinute);
		break;
		
		case 6:
			lcd_gotoxy(12,0);
			lcd_puts_hu(PSTR("ID"));
			lcd_gotoxy(0,1);
			for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
			{
				lcd_putbyte_hex(SecondFloor.SensorID[i]);
			}
		break;
	}
	
}

bool SecondFloor_ActionCallback(MENU_BUTTON *button, uint8_t column)
{
	uint8_t actSensor = FindActSensor(SecondFloor.SensorID);
	switch(button->role){
		case MENU_UP:
			switch (column)
			{
				case 1:
					if (++SecondFloorFracDesired == 10)
					{
						++SecondFloorDesired;
						SecondFloorFracDesired = 0;
					}
				break;
				
				case 2:
					SecondFloorThermostatActive ^= (1 << 0);
					break;
				
				case 3:
					if (++SecondFloorHysteresis == 10)
						SecondFloorHysteresis = 0;					
				break;
				
				case 4:
					if (++SecondFloorMinTimeMinute == 60)
					{
						SecondFloorMinTimeMinute = 0;
						if (++SecondFloorMinTimeHour == 24)
						SecondFloorMinTimeHour = 0;
					}
				break;
				
				case 5:
					if (++SecondFloorMaxTimeMinute == 60)
					{
						SecondFloorMaxTimeMinute = 0;
						if (++SecondFloorMaxTimeHour == 24)
						SecondFloorMaxTimeHour = 0;
					}
				break;
				
				case 6:
					if (++actSensor == nSensors)
						actSensor = 0;
				break;
			}			
		break;
		
		case MENU_DOWN:
			switch (column)
			{
				case 1:
					if (--SecondFloorFracDesired == 255)
					{
						--SecondFloorDesired;
						SecondFloorFracDesired = 9;
					}
				break;
				
				case 2:
					SecondFloorThermostatActive ^= (1 << 0);
				break;
				
				case 3:
					if (++SecondFloorHysteresis == 255)
						SecondFloorHysteresis = 9;
				break;
				
				case 4:
					if (--SecondFloorMinTimeMinute == 255)
					{
						SecondFloorMinTimeMinute = 59;
						if (--SecondFloorMinTimeHour == 255)
						SecondFloorMinTimeHour = 23;
					}
				break;
				
				case 5:
					if (--SecondFloorMaxTimeMinute == 255)
					{
						SecondFloorMaxTimeMinute = 59;
						if (--SecondFloorMaxTimeHour == 255)
						SecondFloorMaxTimeHour = 23;
					}
				break;
				
				case 6:
					if (--actSensor == 255)
						actSensor = nSensors - 1;				
				break;
			}
		
		
		break;
		
		case MENU_CONFIRM:
			switch (column)
			{	
				case 1:
					eeprom_update_byte(&eeSecondFloorDesired, SecondFloorDesired);
					eeprom_update_byte(&eeSecondFloorFracDesired, SecondFloorFracDesired);
					break;
					
				case 2:
					eeprom_update_byte(&eeSecondFloorThermostatActive, SecondFloorThermostatActive);
				break;
					
				case 3:
					eeprom_update_byte(&eeSecondFloorHysteresis, SecondFloorHysteresis);
				break;
				
				case 4:
					eeprom_update_byte(&eeSecondFloorMinTimeHour, SecondFloorMinTimeHour);
					eeprom_update_byte(&eeSecondFloorMinTimeMinute, SecondFloorMinTimeMinute);
				break;
				
				case 5:
					eeprom_update_byte(&eeSecondFloorMaxTimeHour, SecondFloorMaxTimeHour);
					eeprom_update_byte(&eeSecondFloorMaxTimeMinute, SecondFloorMaxTimeMinute);
				break;
				
				case 6:
					eeprom_update_block(gSensorIDs[actSensor], &eeSecondFloorSensorID, OW_ROMCODE_SIZE);
				break;
			}
			return true;
					
		case MENU_CANCEL:
		return true;
	}
	memcpy(&SecondFloor.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	SecondFloor_CallbackRender(column);
	return false;
}

# define SECONDFLOOR_SUBMENU_ITEMS  7
static MENU_ITEM SecondFloor_submenu[SECONDFLOOR_SUBMENU_ITEMS] = {
	{"Felsõ szint hõm", 	SecondFloor_CallbackRender, 	Simple_Callback, 					0,						NULL},
	{"Felsõ szint kivánt",	SecondFloor_CallbackRender,		SecondFloor_ActionCallback,			0,						NULL},
	{"Felsõ szint aktiv",	SecondFloor_CallbackRender,		SecondFloor_ActionCallback,			0,						NULL},
	{"Felsõ szint hiszt.",	SecondFloor_CallbackRender,		SecondFloor_ActionCallback,			0,						NULL},
	{"F. szint min idõ",	SecondFloor_CallbackRender, 	SecondFloor_ActionCallback, 		0, 						NULL},
	{"F. szint max idõ",	SecondFloor_CallbackRender, 	SecondFloor_ActionCallback, 		0, 						NULL},
	{"Felsõ szint ID",		SecondFloor_CallbackRender, 	SecondFloor_ActionCallback, 		0, 						NULL},
};

/*************************************************************************
 Menu Buffer, submenu definitions
*************************************************************************/
void BufferTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Buffer.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Buffer.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(Buffer.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(Buffer.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
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

void BufferMaxTemp_CallbackRender(uint8_t which){
	char buf[4];
	itoa(BufferMaxTemp, buf, 10);
	lcd_clrscr();
	lcd_puts_hu(PSTR("Puffer max hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf);
}

bool BufferMaxTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (BufferMaxTemp != 99)
				++BufferMaxTemp;
			break;
		case MENU_DOWN:
			if (BufferMaxTemp > 60)
				--BufferMaxTemp;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeBufferMaxTemp, BufferMaxTemp);
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
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Buffer.SensorID[i]);
	}
}

bool BufferSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Buffer.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:			
			eeprom_update_block(gSensorIDs[actSensor], &eeBufferSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&Buffer.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	BufferSensor_CallbackRender(column);
	return false;
}

# define BUFFER_SUBMENU_ITEMS  4
static MENU_ITEM BUFFER_submenu[BUFFER_SUBMENU_ITEMS] = {
	{"Puffer akt hõm", 		BufferTempActual_CallbackRender, 	Simple_Callback,		 			0,				NULL},
	{"Elõremenõ hõm", 		ForwardHeatTemp_CallbackRender, 	ForwardHeatTemp_ActionCallback,		0,				NULL},
	{"Puffer max hõm", 		BufferMaxTemp_CallbackRender, 		BufferMaxTemp_ActionCallback,		0,				NULL},
	{"Puffer szenzor",		BufferSensor_CallbackRender, 		BufferSensor_ActionCallback, 		0, 				NULL},
};

/*************************************************************************
 Menu Garage, submenu definitions
*************************************************************************/
void EngineeringTempActual_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gépház akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Engineering.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Engineering.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(Engineering.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(Engineering.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
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
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Engineering.SensorID[i]);
	}
}

bool EngineeringSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Engineering.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeEngineeringSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&Engineering.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	EngineeringSensor_CallbackRender(column);
	return false;
}

# define ENGINEERING_SUBMENU_ITEMS  4
static MENU_ITEM ENGINEERING_submenu[ENGINEERING_SUBMENU_ITEMS] = {
	{"Gépház akt hõm", 		EngineeringTempActual_CallbackRender, 	Simple_Callback,				 		0,				NULL},
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
			Relays ^= (1 << DHW_VALVE_RELAY);
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
	lcd_puts_hu(PSTR("Földszinti szivattyú"));
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
	lcd_puts_p(PSTR("Emeleti szivattyú"));
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
	{"Földszint sziv", 			ZoneValve1_CallbackRender, 			ZoneValve1_ActionCallback, 		0,	NULL},
	{"Emelet sziv", 			ZoneValve2_CallbackRender, 			ZoneValve2_ActionCallback, 		0,	NULL},
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
	lcd_puts(Garage.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Garage.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(Garage.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(Garage.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

void GarageSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Garázs szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Garage.SensorID[i]);
	}
}

bool GarageSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Garage.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeGarageSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&Garage.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	GarageSensor_CallbackRender(column);
	return false;
}

void LivingRoomTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Nappali akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(LivingRoom.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(LivingRoom.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(LivingRoom.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(LivingRoom.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

void LivingRoomSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Nappali szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(LivingRoom.SensorID[i]);
	}
}

bool LivingRoomSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(LivingRoom.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeLivingRoomSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&LivingRoom.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	LivingRoomSensor_CallbackRender(column);
	return false;
}

void FloorTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Padló akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Floor.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Floor.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));

	char buffer[7];
	lcd_gotoxy(0,2);
	itoa(Floor.MeasuredMinimumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	itoa(Floor.MeasuredMaximumTemp, buffer, 10);
	lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

void FloorSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Padló szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Floor.SensorID[i]);
	}
}

bool FloorSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Floor.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeFloorSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}

	memcpy(&Floor.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	FloorSensor_CallbackRender(column);
	return false;
}

void GasForwardTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Elõre akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(GasForward.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(GasForward.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
}

void GasForwardSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Elõre szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(GasForward.SensorID[i]);
	}
}

bool GasForwardSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(GasForward.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeGasForwardTempSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}

	memcpy(&GasForward.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	GasForwardSensor_CallbackRender(column);
	return false;
}

void GasReturnTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Visszatérõ akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(GasReturn.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(GasReturn.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
}

void GasReturnSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Vissza szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(GasReturn.SensorID[i]);
	}
}

bool GasReturnSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(GasReturn.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeGasReturnTempSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}

	memcpy(&GasReturn.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	GasReturnSensor_CallbackRender(column);
	return false;
}

void ForwardTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Elõre akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Forward.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Forward.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
}

void ForwardSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Elõre szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Forward.SensorID[i]);
	}
}

bool ForwardSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Forward.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeForwardTempSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}
	memcpy(&Forward.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	ForwardSensor_CallbackRender(column);
	return false;
}

void ReturnTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Visszatérõ akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Return.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Return.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
}

void ReturnSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Vissza szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Return.SensorID[i]);
	}
}

bool ReturnSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Return.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeReturnTempSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}

	memcpy(&Return.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	ReturnSensor_CallbackRender(column);
	return false;
}

void MixedTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Kevert akt hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(Mixed.ActualTempBuffer); lcd_puts_p(PSTR(".")), lcd_puts(Mixed.ActualTempFracBuffer); lcd_puts_p(PSTR(" C"));
}

void MixedSensor_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Kevert szenzor ID"));
	lcd_gotoxy(0,1);
	for (uint8_t i = 0; i < OW_ROMCODE_SIZE; i++)
	{
		lcd_putbyte_hex(Mixed.SensorID[i]);
	}
}

bool MixedSensor_ActionCallback(MENU_BUTTON *button, uint8_t column){
	uint8_t actSensor = FindActSensor(Mixed.SensorID);
	switch(button->role){
		case MENU_UP:
			if (++actSensor == nSensors)
				actSensor = 0;
			break;
		case MENU_DOWN:
			if (--actSensor == 255)
				actSensor = nSensors - 1;
			break;
		case MENU_CONFIRM:
			eeprom_update_block(gSensorIDs[actSensor], &eeMixedTempSensorID, OW_ROMCODE_SIZE);
			return true;
		case MENU_CANCEL:
			return true;
	}

	memcpy(&Mixed.SensorID, &gSensorIDs[actSensor], OW_ROMCODE_SIZE);
	MixedSensor_CallbackRender(column);
	return false;
}

# define SENSORS_SUBMENU_ITEMS  16
static MENU_ITEM SENSORS_submenu[SENSORS_SUBMENU_ITEMS] = {
	{"Garázs hõm",			GarageTemp_CallbackRender,			Simple_Callback, 					0, NULL},
	{"Garázs szenzor ID",	GarageSensor_CallbackRender,		GarageSensor_ActionCallback,		0, NULL},
	{"Nappali hõm",			LivingRoomTemp_CallbackRender,		Simple_Callback,					0, NULL},
	{"Nappali szenzor ID",	LivingRoomSensor_CallbackRender,	LivingRoomSensor_ActionCallback,	0, NULL},
	{"Padló hõm",			FloorTemp_CallbackRender,			Simple_Callback,					0, NULL},
	{"Padló szenzor ID",	FloorSensor_CallbackRender,			FloorSensor_ActionCallback,			0, NULL},
	{"Gáz elõre hõm",		GasForwardTemp_CallbackRender,		Simple_Callback,					0, NULL},
	{"Gáz elõre ID",		GasForwardSensor_CallbackRender,	GasForwardSensor_ActionCallback,	0, NULL},
	{"Gáz vissza hõm",		GasReturnTemp_CallbackRender,		Simple_Callback,					0, NULL},
	{"Gáz vissza ID",		GasReturnSensor_CallbackRender,		GasReturnSensor_ActionCallback,		0, NULL},
	{"Elõre hõm",			ForwardTemp_CallbackRender,			Simple_Callback,					0, NULL},
	{"Elõre ID",			ForwardSensor_CallbackRender,		ForwardSensor_ActionCallback,		0, NULL},
	{"Vissza hõm",			ReturnTemp_CallbackRender,			Simple_Callback,					0, NULL},
	{"Vissza ID",			ReturnSensor_CallbackRender,		ReturnSensor_ActionCallback,		0, NULL},
	{"Kevert hõm",			MixedTemp_CallbackRender,			Simple_Callback,					0, NULL},
	{"Kevert ID",			MixedSensor_CallbackRender,			MixedSensor_ActionCallback,		0, NULL},
};


/*************************************************************************
 Menu Settings, submenu definition
*************************************************************************/
void Mode_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Tesztüzem"));
	lcd_gotoxy(0,1);
	switch (Mode)
	{
		case 0:
			lcd_puts_hu(PSTR("Automatikus"));
			break;
		
		case 1:
			lcd_puts_hu(PSTR("Manuális"));
			break;
		
		case 2:
			lcd_puts_hu(PSTR("Hibakeresés"));
			break;		
	}
}

bool Mode_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++Mode == 3)
				Mode = 0;
			break;
		case MENU_DOWN:
			if (--Mode == 255)
				Mode = 2;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeMode, Mode);
			return true;
		case MENU_CANCEL:
			return true;
	}

	Mode_CallbackRender(column);
	return false;
}

void MenuTimer_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Menü idõzítõ (mp)"));
	char buf[4];
	lcd_gotoxy(0,1);
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

void GasForwardHeatTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Gáz OT elõre"));
	lcd_gotoxy(0,1);
	char buf[4];
	itoa(GasForwardHeatTemp, buf, 10);
	lcd_puts(buf);
}

bool GasForwardHeatTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			++GasForwardHeatTemp;
			break;
		case MENU_DOWN:
			--GasForwardHeatTemp;
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeGasForwardHeatTemp, GasForwardHeatTemp);
			return true;
		case MENU_CANCEL:
			return true;
	}

	GasForwardHeatTemp_CallbackRender(column);
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
	if (ComfortMode == 0)
		lcd_puts_p(PSTR("Kikapcsolva"));
	else if (ComfortMode == 1)
		lcd_puts_hu(PSTR("Alsó szint"));
	else if (ComfortMode == 2)
		lcd_puts_hu(PSTR("Felsõ szint"));
	else if (ComfortMode == 3)
		lcd_puts_hu(PSTR("Mindkét szint"));
	else if (ComfortMode == 4)
		lcd_puts_hu(PSTR("Padló"));
}

bool ComfortMode_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortMode > 4)
				ComfortMode = 0;
		break;
		case MENU_DOWN:
			if (--ComfortMode == 255)
				ComfortMode = 4;
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
	char buf1[5];
	char buf2[2];
	itoa(ComfortTemp, buf1, 10);
	strcat(buf1, ".");
	itoa(ComfortTempFrac, buf2, 10);
	strcat(buf1, buf2);
	
	lcd_clrscr();
	lcd_puts_hu(PSTR("Komfort hõm"));
	lcd_gotoxy(0,1);
	lcd_puts(buf1);
}

bool ComfortTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortTempFrac == 10)
			{
				ComfortTemp++;
				ComfortTempFrac = 0;
			}
			break;
		case MENU_DOWN:
			if (--ComfortTempFrac == 0xFF)
			{
				ComfortTemp--;
				ComfortTempFrac = 9;
			}
			break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortTemp, ComfortTemp);
			eeprom_update_byte(&eeComfortTempFrac, ComfortTempFrac);
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
	char buf[5];
	itoa(ComfortForwardTemp, buf, 10);
	lcd_puts(buf);
}

bool ComfortForwardTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortForwardTemp == ForwardHeatTemp)
				ComfortForwardTemp--;
			break;
		case MENU_DOWN:
			if (--ComfortForwardTemp == 10)
				ComfortForwardTemp++;
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

void ComfortFloorTemp_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Komfort padló"));
	lcd_gotoxy(0,1);
	char buf[5];
	itoa(ComfortFloorTemp, buf, 10);
	lcd_puts(buf);
}

bool ComfortFloorTemp_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		if (++ComfortFloorTemp == ForwardHeatTemp)
		ComfortFloorTemp--;
		break;
		case MENU_DOWN:
		if (--ComfortFloorTemp == 10)
		ComfortFloorTemp++;
		break;
		case MENU_CONFIRM:
		eeprom_update_byte(&eeComfortFloorTemp, ComfortFloorTemp);
		return true;
		case MENU_CANCEL:
		return true;
	}

	ComfortFloorTemp_CallbackRender(column);
	return false;
}

void ComfortMinTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Min idõ"));
	lcd_gotoxy(0,1);	
	lcd_puti(ComfortMinTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(ComfortMinTimeMinute);
}

bool ComfortMinTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
			if (++ComfortMinTimeMinute == 60)
			{
				ComfortMinTimeMinute = 0;
				if (++ComfortMinTimeHour == 24)
				ComfortMinTimeHour = 0;
			}
		break;
		case MENU_DOWN:
			if (--ComfortMinTimeMinute == 255)
			{
				ComfortMinTimeMinute = 59;
				if (--ComfortMinTimeHour == 255)
					ComfortMinTimeHour = 23;
			}
		break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortMinTimeHour, ComfortMinTimeHour);
			eeprom_update_byte(&eeComfortMinTimeMinute, ComfortMinTimeMinute);
		case MENU_CANCEL:
		return true;
	}

	ComfortMinTime_CallbackRender(column);
	return false;
}

void ComfortMaxTime_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Max idõ"));
	lcd_gotoxy(0,1);	
	lcd_puti(ComfortMaxTimeHour); lcd_puts_p(PSTR(":")); lcd_puti(ComfortMaxTimeMinute);
}

bool ComfortMaxTime_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		if (++ComfortMaxTimeMinute == 60)
		{
			ComfortMaxTimeMinute = 0;
			if (++ComfortMaxTimeHour == 24)
			ComfortMaxTimeHour = 0;
		}
		break;
		case MENU_DOWN:
			if (--ComfortMaxTimeMinute == 255)
			{
				ComfortMaxTimeMinute = 59;
				if (--ComfortMaxTimeHour == 255)
					ComfortMaxTimeHour = 23;
			}
		break;
		case MENU_CONFIRM:
			eeprom_update_byte(&eeComfortMaxTimeHour, ComfortMaxTimeHour);
			eeprom_update_byte(&eeComfortMaxTimeMinute, ComfortMaxTimeMinute);
		case MENU_CANCEL:
		return true;
	}

	ComfortMaxTime_CallbackRender(column);
	return false;
}

#define COMFORT_SUBMENU_ITEMS 6
static MENU_ITEM COMFORT_submenu[COMFORT_SUBMENU_ITEMS] = {
	{"Komfort mód", 			ComfortMode_CallbackRender, 		ComfortMode_ActionCallback,			0,	NULL},
	{"Komfort hõm", 			ComfortTemp_CallbackRender,			ComfortTemp_ActionCallback,			0,	NULL},
	{"Komfort elõremenõ", 		ComfortForwardTemp_CallbackRender,	ComfortForwardTemp_ActionCallback,	0,	NULL},
	{"Komfort padló", 			ComfortFloorTemp_CallbackRender,	ComfortFloorTemp_ActionCallback,	0,	NULL},
	{"Komfort min idõ", 		ComfortMinTime_CallbackRender,		ComfortMinTime_ActionCallback,		0,	NULL},
	{"Komfort max idõ", 		ComfortMaxTime_CallbackRender,		ComfortMaxTime_ActionCallback,		0,	NULL}
};

void Restarts_CallbackRender(uint8_t which){
	lcd_clrscr();
	lcd_puts_hu(PSTR("Újraindulások"));
	lcd_gotoxy(0,1);
	char buf[6];
	itoa(Restarts, buf, 10);
	lcd_puts(buf);

	//char buffer[7];
	//lcd_gotoxy(0,2);
	//ftoa(buffer, HIHxxxxTempMin, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR("/"));
	//ftoa(buffer, HIHxxxxTempMax, 2);
	//lcd_puts(buffer); lcd_puts_p(PSTR(" C"));
}

bool Restarts_ActionCallback(MENU_BUTTON *button, uint8_t column){
	switch(button->role){
		case MENU_UP:
		case MENU_DOWN:
		case MENU_CONFIRM:
			eeprom_update_word(&eeRestarts, 0);
			Restarts = 0;
			break;
		case MENU_CANCEL:
		return true;
	}
	return false;
}

#define SYSPARAM_SUBMENU_ITEMS 8
static MENU_ITEM SYSPARAM_submenu[SYSPARAM_SUBMENU_ITEMS] = {
	{"Szivattyú után", 	PumpPlusTime_CallbackRender, 		PumpPlusTime_ActionCallback,	0,						NULL},
	{"Gáz OT elõre", 	GasForwardHeatTemp_CallbackRender, 	GasForwardHeatTemp_ActionCallback,	0,						NULL},
	{"Komfort mód", 	NULL,						 		NULL,							COMFORT_SUBMENU_ITEMS,	COMFORT_submenu},
	{"óra, dátum", 		NULL,				 				NULL,							CLOCK_SUBMENU_ITEMS,	CLOCK_submenu},
	{"Üzemmód", 		Mode_CallbackRender, 				Mode_ActionCallback, 			0,						NULL},
	{"Menü idõzítõ", 	MenuTimer_CallbackRender, 			MenuTimer_ActionCallback, 		0,						NULL},
	{"LCD háttér", 		LCDBackLight_CallbackRender, 		LCDBackLight_ActionCallback,	0,						NULL},
	{"Újraindulások",	Restarts_CallbackRender, 			Restarts_ActionCallback,		0,						NULL},
};

/*
** HOME menu items definition
*/
#define MENU_HOME_ITEMS  9
static MENU_ITEM home_items[MENU_HOME_ITEMS] = {
	{"HMV beállítás",   	NULL,                           NULL,                     DHW_SUBMENU_ITEMS,     		DHW_submenu	  	},
	{"Felsõ szint",   		NULL,                           NULL,                     SECONDFLOOR_SUBMENU_ITEMS,    SecondFloor_submenu	},	
	{"Puffer beállítás",   	NULL,                           NULL,                     BUFFER_SUBMENU_ITEMS,     	BUFFER_submenu	},
	{"Gépház termoszt",		NULL,                           NULL,                     ENGINEERING_SUBMENU_ITEMS,	ENGINEERING_submenu	},
	{"Külsõ érzékelõ",		NULL,                           NULL,                     HIHxxxx_SUBMENU_ITEMS,   		HIHxxxx_submenu	},
	{"Vegyes kazán",		NULL,                           NULL,                     Wood_SUBMENU_ITEMS,   		Wood_submenu	},
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