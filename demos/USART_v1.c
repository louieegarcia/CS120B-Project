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
#include "USART.h"
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

enum USART_STATES {WAIT, SEND, RECEIVE};

char command[] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x03,0x01,0x00,0x05};

unsigned char ReadySend = 0;
unsigned char ReadyRecv = 0;
unsigned char Transmitted = 0;
unsigned char cmd = 0x01;
unsigned char returned = 0x00;

int USART_tick(int state){
	switch(state){
		case WAIT:
			state = (button1)?(SEND):(WAIT);
			break;
		case SEND:
			if(Transmitted){
				state = RECEIVE;
				ReadySend = 0;
			} else{
				state = SEND;
				cmd = 0x01;
			}
			break;
		case RECEIVE:
			if(ReadyRecv){
				state = WAIT;
				ReadyRecv = 0;
			} else{
				state = RECEIVE;
			}
			break;
		default:
			state = WAIT;
	}

	switch(state){
		case WAIT:
			LCD_DisplayString(1,"Waiting...");
			break;
		case SEND:
			LCD_DisplayString(1,"Sending...");
			ReadySend = USART_IsSendReady();
			if(ReadySend){
				LCD_DisplayString(1,"Sending String");
				USART_SendString(command, sizeof(command));	
				Transmitted = USART_HasTransmitted();
			}
			break;
		case RECEIVE:
			LCD_DisplayString(1,"Receiving...");
			ReadyRecv = USART_HasReceived();
		       	if(!ReadyRecv){
				returned = USART_Receive();
			}
			break;
	}
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	task task1;
	task1.state = WAIT;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &USART_tick;

	task* tasklist[] = {&task1};

    	TimerOn();
	TimerSet(200);	
	LCD_init();
	USART_Init(BAUD_RATE);

	unsigned char i;
	while (1) {
		for(i = 0; i < 1; i++){
			if ( tasklist[i]->elapsedTime == tasklist[i]->period ) {
				tasklist[i]->state = tasklist[i]->TickFct(tasklist[i]->state);
				tasklist[i]->elapsedTime = 0;
			}
			tasklist[i]->elapsedTime += 100;
		}
			
		while(!TimerFlag);
		TimerFlag = 0;
    	}
    	return 1;
}
