/*	Author: lgarc038
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
#include "io.h"
#include "timer.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#define button1 (~PINA & 0x01)
#define button2 (~PINA & 0x02)
#define button3 (~PINA & 0x04)
#define button4 (~PINA & 0x08)

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct) (int);
} task;

char alpha[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
unsigned char alphaIndex = 0;
unsigned char cursor = 1;
unsigned char string[16];
unsigned char tempString[16];

enum MENU_States {MENU_WAIT,MENU_INC,MENU_DEC,MENU_WRITE,MENU_CLEAR,MENU_EEPROM_WRITE_PRESS,MENU_EEPROM_WRITE_RELEASE,MENU_EEPROM_READ,SMILEY};

int Tick_MENU(int state){
	switch(state){
		case MENU_WAIT:
			if(button1 && !button2 && !button3 && !button4) state = MENU_WRITE;
			else if(!button1 && button2 && !button3 && !button4) state = MENU_DEC;
			else if(!button1 && !button2 && button3 && !button4) state = MENU_INC;
			else if(!button1 && button2 && button3 && !button4) state = MENU_CLEAR;
			else if(button1 && button2 && !button3 && !button4){state = MENU_EEPROM_READ; strcpy(tempString,string);}
			else if(!button1 && !button2 && !button3 && button4) state = MENU_EEPROM_WRITE_PRESS;
			else if(button1 && !button2 && !button3 && button4){state = SMILEY; strcpy(tempString,string);}
			else state = MENU_WAIT;
			break;
		case MENU_INC:
			state = MENU_WAIT;
			break;
		case MENU_DEC:
			state = MENU_WAIT;
			break;
		case MENU_WRITE:
			state = MENU_WAIT;
			break;
		case MENU_CLEAR:
			state = MENU_WAIT;
			break;
		case MENU_EEPROM_WRITE_PRESS:
			state = (button4)?(MENU_EEPROM_WRITE_PRESS):(MENU_EEPROM_WRITE_RELEASE);
			break;
		case MENU_EEPROM_WRITE_RELEASE:
			state = MENU_WAIT;
			break;
		case MENU_EEPROM_READ:
			state = (button1 && button2 && !button3)?(MENU_EEPROM_READ):(MENU_WAIT);
			if(state == MENU_WAIT){memset(string,0,16); strcpy(string,tempString);}
			break;
		case SMILEY:
			state = (button1 && button4)?(SMILEY):(MENU_WAIT);
			if(state == MENU_WAIT){memset(string,0,16); strcpy(string,tempString);}
			break;
		default:
			state = MENU_WAIT;
			break;
	}

	switch(state){
		case MENU_WAIT:
			break;
		case MENU_INC:
			if(alphaIndex < 51) alphaIndex++;
			string[cursor-1] = alpha[alphaIndex];
			break;
		case MENU_DEC:
			if(alphaIndex > 0) alphaIndex--;
			string[cursor-1] = alpha[alphaIndex];
			break;
		case MENU_WRITE:
			string[cursor - 1] = alpha[alphaIndex];
			if(cursor < 16) cursor++;
			break;
		case MENU_CLEAR:
			memset(string,0,16);
			alphaIndex = 0;
			cursor = 1;
			break;
		case MENU_EEPROM_WRITE_PRESS:
			break;
		case MENU_EEPROM_WRITE_RELEASE:
			eeprom_write_block((void*)&string,(const void*)12,16);
			break;
		case MENU_EEPROM_READ:
			eeprom_read_block((void*)&string,(const void*)12,16);
			break;
		case SMILEY:
			strcpy(string,":)");
			break;
	}

	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	task task1;
	task1.state = MENU_WAIT;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Tick_MENU;

	task* tasklist[] = {&task1};

    	TimerOn();
	TimerSet(100);	
	LCD_init();
	string[0] = alpha[0];

	unsigned char i;
	while (1) {
		for(i = 0; i < 1; i++){
			if ( tasklist[i]->elapsedTime == tasklist[i]->period ) {
				tasklist[i]->state = tasklist[i]->TickFct(tasklist[i]->state);
				tasklist[i]->elapsedTime = 0;
			}
			tasklist[i]->elapsedTime += 100;
		}

		LCD_DisplayString(1,string);
		if(!button1 && !button2){
			LCD_Cursor(cursor);
			LCD_WriteData(alpha[alphaIndex]);
		}
		while(!TimerFlag);
		TimerFlag = 0;
    	}
    	return 1;
}
