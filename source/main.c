#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
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

typedef struct user{
	char name[10];
	unsigned char id;
} user;

user userList[5];
unsigned char userListIndex = 0;
unsigned char deleteMenuIndex = 0;
unsigned char userListSize = 0;

enum states {
MENU,STARTUP,TITLE,ADDUSER,DELUSER,AUTHUSER,AUTHMODE,ADDUSER_INC_PRESS,ADDUSER_INC_REL,
ADDUSER_DEC_PRESS, ADDUSER_DEC_REL, ADDUSER_CHOOSE, DELUSER_INC,DELUSER_DEC,DELUSER_CONFIRM,DELUSER_DELETE
};
char alpha[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
unsigned char alphaIndex = 0;
unsigned long SMPeriod = 100;
unsigned char cursorX = 0;
unsigned char inputIndex = 0;
char input[10];

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

void deleteUserMenu(){
	nokia_lcd_clear();
	nokia_lcd_set_cursor(10,10);
	nokia_lcd_write_string(userList[deleteMenuIndex].name,1);
	nokia_lcd_render();
}

void addUserInput(){
		nokia_lcd_clear();
		nokia_lcd_set_cursor(0,0);
		nokia_lcd_write_string("Input Name: ",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string(input,1);
		nokia_lcd_set_cursor(0,20);
		nokia_lcd_write_char(alpha[alphaIndex],1);
		nokia_lcd_render();
}

void addUserToList(char input[10], unsigned char i){
	strcpy(userList[i].name,input);	
	userList[i].id = i;
}

int tick(int state){
	static unsigned char count = 0;
	static unsigned char menuItem = 10;

	switch(state){
		case MENU:
			// BUTTON3 pushed, if menuItem equals 10,20,30,40 then choose corresponding mode
			if(BUTTON3){
				if(menuItem == 10){
					state = ADDUSER;
					memset(input,0,sizeof(input));
				} else if(menuItem == 20){
					state = DELUSER;
				} else if(menuItem == 30){
					state = AUTHUSER;
				} else if(menuItem == 40){
					state = AUTHMODE;
				} else{
					state = MENU;
				}
			} else{
				state = MENU;
			}
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
		case ADDUSER:
			if(BUTTON1){
				state = ADDUSER_DEC_PRESS;
			} else if(BUTTON2){
				state = ADDUSER_INC_PRESS;
			} else if(BUTTON3){
				state = ADDUSER_CHOOSE;
			} else if(BUTTON4){
				state = MENU;
				alphaIndex = 0;
				cursorX = 0;
				inputIndex = 0;
				addUserToList(input, userListIndex);
			} else{
				state = ADDUSER;
			}
			break;
		case ADDUSER_DEC_PRESS:
			state = (!BUTTON1)?(ADDUSER_DEC_REL):(ADDUSER_DEC_PRESS);
			break;
		case ADDUSER_DEC_REL:			
			state = ADDUSER;
			break;
		case ADDUSER_INC_PRESS:
			state = (!BUTTON2)?(ADDUSER_INC_REL):(ADDUSER_INC_PRESS);
			break;
		case ADDUSER_INC_REL:
			state = ADDUSER;
			break;
		case ADDUSER_CHOOSE:
			state = ADDUSER;
			break;
		case DELUSER:
			if(BUTTON1){
				state = DELUSER_DEC;
			} else if(BUTTON2){
				state = DELUSER_INC;
			} else if(BUTTON3){
				state = DELUSER_CONFIRM;
			} else if(BUTTON4){
				state = MENU;
			} else{
				state = DELUSER;
			}
			break;
		case DELUSER_INC:
			state = DELUSER;
			break;
		case DELUSER_DEC:
			state = DELUSER;
			break;
		case DELUSER_CONFIRM:
			if(BUTTON3){
				state = DELUSER_DELETE;
			} else if(BUTTON4){
				state = DELUSER;
			} else{
				state = DELUSER_CONFIRM;
			}
			break;
		case DELUSER_DELETE:
			state = MENU;
			break;
		case AUTHUSER:
			state = (BUTTON4)?(MENU):(AUTHUSER);
			break;
		case AUTHMODE:
			state = (BUTTON4)?(MENU):(AUTHMODE);
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
		case ADDUSER:
			addUserInput();
			break;
		case ADDUSER_INC_PRESS:
			break;
		case ADDUSER_INC_REL:
			if(alphaIndex < 51) alphaIndex++;
			break;
		case ADDUSER_DEC_PRESS:
			break;
		case ADDUSER_DEC_REL:
			if(alphaIndex > 0) alphaIndex--;
			break;
		case ADDUSER_CHOOSE:
			input[inputIndex] = alpha[alphaIndex];
			if(inputIndex<10) inputIndex++;
			if(userListSize<4) userListSize++;
			break;
		case DELUSER:
			if(userListSize == 0){
				nokia_lcd_clear();
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("No users to delete. Please go back to the menu.",1);
				nokia_lcd_render();
			}
			deleteUserMenu();			
			break;
		case DELUSER_INC:
			if(deleteMenuIndex < userListSize) deleteMenuIndex++;
			break;
		case DELUSER_DEC:
			if(deleteMenuIndex > 0) deleteMenuIndex--;
			break;
		case DELUSER_CONFIRM:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0,10);
			nokia_lcd_write_string("Are you sure you want to delete this user?",1);
			nokia_lcd_render();
			break;
		case DELUSER_DELETE:
			break;
		case AUTHUSER:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(10,10);
			nokia_lcd_write_string("AUTHUSER",1);
			nokia_lcd_render();			
			break;
		case AUTHMODE:
			nokia_lcd_clear();
			nokia_lcd_set_cursor(10,10);
			nokia_lcd_write_string("AUTHMODE",1);
			nokia_lcd_render();
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

	unsigned char i;
	while(1){
		for(i = 0; i < 1; i++){
			if(tasklist[i]->elapsedTime >= tasklist[i]->period){
				tasklist[i]->state = tasklist[i]->TickFct(tasklist[i]->state);
				tasklist[i]->elapsedTime = 0;
			}
			
			tasklist[i]->elapsedTime += SMPeriod;
		}

		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 1;
}
