#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <avr/io.h>


/*
* General menu buttons
*/
#define MENU_PORTSIO_DDR					DDRC				// used for buttons in PCB
#define MENU_PORTSIO_PIN					PINC
#define MENU_PORTSIO_PORT	     		    PORTC
#define MENU_PORTSIO_BTN_UP	        	    PORTC2				// button UP
#define MENU_PORTSIO_BTN_DOWN		        PORTC4				// button DOWN
#define MENU_PORTSIO_BTN_CONFIRM	        PORTC5      		// button CONFIRM
#define MENU_PORTSIO_BTN_CANCEL	            PORTC3      		// button CANCEL


void menuPortsioInit();

#endif /* SETTINGS_H_ */
