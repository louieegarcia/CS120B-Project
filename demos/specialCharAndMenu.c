#include <avr/io.h>
#include <avr/interrupt.h>
#include "nokia5110.h"
#include "timer.h"

#define BUTTON1 (~PINA & 0x01)
#define BUTTON2 (~PINA & 0x02)
#define BUTTON3 (~PINA & 0x04)
#define BUTTON4 (~PINA & 0x08)

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct) (int);
} task;

enum states {MENU,STARTUP,TITLE};
unsigned long SMPeriod = 100;

void PRINT_OPEN_EYE_FACE(unsigned char jump){
	/***************************
	* LCD Dimensions: 84 x 48  *
	* Start at 24 x 0, and     *
	* write the pixels in from *
	* 12 - 36 and 28 - 56      *
	***************************/
	unsigned char arr[] = {
		20,30,100,19,30,100,18,30,100,17,30,100,16,30,100,15,30,100,14,30,100,13,30,100,12,30,100,11,30,100,10,30,100,
		11,30,100,30,12,100,13,30,100,14,30,100,15,30,100,16,30,100,17,30,100,18,30,100,19,30,100,20,30,100,
		30,100,30,100,30,100,30,100,
		20,30,100,19,30,100,18,30,100,17,30,100,16,30,100,15,30,100,14,30,100,13,30,100,12,30,100,11,30,100,10,30,100,
		11,30,100,30,12,100,13,30,100,14,30,100,15,30,100,16,30,100,17,30,100,18,30,100,19,30,100,20,30,100,
	};
	unsigned char i = 21;
	unsigned char index = 0;

	nokia_lcd_clear();

	while(i < 67){
		if(arr[index] == 100){
			i++;
			index++;
		} else if(arr[index] == 101){
			i+=4;
			index++;
		} else{
			nokia_lcd_set_pixel(i,arr[index%sizeof(arr)]-jump,i);
			index++;
		}
	}

	nokia_lcd_render();
}

void PRINT_CLOSE_EYE_FACE(){
	unsigned char arr[] = {
		20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,
		20,30,100,30,20,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,
		30,100,30,100,30,100,30,100,
		20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,
		20,30,100,30,20,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,20,30,100,
	};

	unsigned char i = 21;
	unsigned char index = 0;

	nokia_lcd_clear();

	while(i < 67){
		if(arr[index] == 100){
			i++;
			index++;
		} else if(arr[index] == 101){
			i+=4;
			index++;
		} else{
			nokia_lcd_set_pixel(i,arr[index%sizeof(arr)],i);
			index++;
		}
	}

	nokia_lcd_render();
}

void printMenu(unsigned char x){
	nokia_lcd_clear();
	nokia_lcd_set_cursor(31,0);
	nokia_lcd_write_string("Menu",1);
	nokia_lcd_set_cursor(15,10);
	nokia_lcd_write_string("Add User",1);
	nokia_lcd_set_cursor(15,20);
	nokia_lcd_write_string("Del User",1);
	nokia_lcd_set_cursor(15,30);
	nokia_lcd_write_string("Auth User",1);
	nokia_lcd_set_cursor(15,40);
	nokia_lcd_write_string("Auth Mode",1);
	nokia_lcd_set_cursor(0,x);
	nokia_lcd_write_string("*",1);
	nokia_lcd_render();
}

int tick(int state){
	static unsigned char count = 0;
	static unsigned char menuItem = 10;

	switch(state){
		case MENU:
			// BUTTON3 pushed, if menuItem equals 10,20,30,40 then choose corresponding mode
			break;
		case STARTUP:
			if(count == 11) state = TITLE;
			else state = STARTUP;
			break;
		case TITLE:
			state = (count == 20)?(MENU):(TITLE);
			if(state == MENU){
				nokia_lcd_clear();
				count = 0;
			}
			break;
		default:
			state = MENU;
			break;
	}

	switch(state){
		case MENU:
			if(BUTTON1){
				if(menuItem >10){
					menuItem -= 10;
				}
			} else if(BUTTON2){
				if(menuItem < 40){
					menuItem += 10;
				}
			}
			
			printMenu(menuItem);
			break;
		case STARTUP:
			if(count >= 5){
				if(count%2 == 0){
					PRINT_OPEN_EYE_FACE(0);
				} else{
					PRINT_OPEN_EYE_FACE(4);
				}
			} else if(count % 2 == 0){
				PRINT_OPEN_EYE_FACE(0);
			} else{
				PRINT_CLOSE_EYE_FACE();
			}
			count++;
			if(count == 11){
				nokia_lcd_set_cursor(12,40);
				nokia_lcd_write_string("BioSecurity",1);
				nokia_lcd_render();
			}
			break;
		case TITLE:
			count++;
			break;
	}

	return state;
}

int main(){
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;

	task task1;
	task1.state = STARTUP;
	task1.period = 200;
	task1.elapsedTime = task1.period;
	task1.TickFct = &tick;

	task* tasklist[] = {&task1};

	nokia_lcd_init();

	TimerOn();
	TimerSet(SMPeriod);

	int stateHold = 0;

	unsigned char i;
	while(1){
		for(i = 0; i < 1; i++){
			if(tasklist[i]->elapsedTime >= tasklist[i]->period){
				tasklist[i]->state = tasklist[i]->TickFct(tasklist[i]->state);
				tasklist[i]->elapsedTime = 0;
			}
			
			tasklist[i]->elapsedTime += SMPeriod;
		}

		//stateHold = tick(stateHold);

		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 1;
}
