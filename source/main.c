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
#include "io.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#define button1 (~PINA & 0x01)
#define button2 (~PINA & 0x02)
#define button3 (~PINA & 0x04)

char alpha[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    
	LCD_init();

    while (1) {
		if(button1){
			LCD_DisplayString(1,"A");
		} else if(button2){
			LCD_DisplayString(1,"B");
		} else if(button3){
			LCD_DisplayString(1,"C");
		} else{
			LCD_DisplayString(1,"waiting");
		}
    }
    return 1;
}
