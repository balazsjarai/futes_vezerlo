#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <avr/io.h>


/*
* General menu buttons
*/
#define MENU_PORTSIO_DDR					DDRD				// used for buttons in PCB
#define MENU_PORTSIO_PIN					PIND
#define MENU_PORTSIO_PORT	     		    PORTD
#define MENU_PORTSIO_BTN_UP	        	    PORTD7  				// button UP
#define MENU_PORTSIO_BTN_DOWN		        PORTD6  				// button DOWN
#define MENU_PORTSIO_BTN_CONFIRM	        PORTD4        		// button CONFIRM
#define MENU_PORTSIO_BTN_CANCEL	            PORTD5        		// button CANCEL

#define BUZZER_DDR		DDRC
#define BUZZER_PORT		PORTC
#define BUZZER_PIN		PINC0


void menuPortsioInit();

#endif /* SETTINGS_H_ */
