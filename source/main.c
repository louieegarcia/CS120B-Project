//#include "mapGeneration.h"

//#include "global.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
#include "io.h"
//#include "timer.h"
//#include "USART.h"
#include "nokia5110.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#define BUTTON1 (~PINA & 0x01)
#define BUTTON2 (~PINA & 0x02)
#define BUTTON3 (~PINA & 0x04)
#define BUTTON4 (~PINA & 0x08)
#define OBSTACLES_SIZE 12
#define TASKS_SIZE 3

unsigned char START = 0;
unsigned char END = 0;
unsigned char SPEED = 2;

typedef struct object{
	unsigned char x;
	unsigned char y;
	unsigned char sym;
} object;

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct) (int);
} task;

unsigned long clockTick = 100;
object obstacles[OBSTACLES_SIZE]; 

// Timer Stuff
unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

// Map Generation Stuff
enum mapStates {MAP_WAIT,MAP_GENERATE};

void MapInit(){
	// X range: 0 - 79
	// Y range: 0 - 42, but we always want to create all the symbols on one line ( Y = 42 )
	unsigned char i = 0;
	while(i < OBSTACLES_SIZE){
		obstacles[i].sym = '=';
		obstacles[i].x = rand() % 80;
		obstacles[i].y = 42;
		i++;
	}
}

void generateMap(){
	unsigned char i = 0;

	while(i < OBSTACLES_SIZE){
		obstacles[i].y -= SPEED;
		nokia_lcd_set_cursor(obstacles[i].x,obstacles[i].y);
		nokia_lcd_write_char(obstacles[i].sym,1);
		if(obstacles[i].y <= 251 && obstacles[i].y >= 245){
			obstacles[i].y = 42;
			obstacles[i].x = rand() % 80;
		}
		i++;
	}
}

int Map_Tick(int state){
	switch(state){
		case MAP_WAIT:
			state = (START)?(MAP_GENERATE):(MAP_WAIT);
			break;
		case MAP_GENERATE:
			state = (END)?(MAP_WAIT):(MAP_GENERATE);
			break;
		default:
			state = MAP_WAIT;
			break;
	}

	switch(state){
		case MAP_WAIT:
			END = 0;
			break;
		case MAP_GENERATE:
			generateMap();
			break;
	}

	return state;
}

// User (Robbers) and Mob(Cops)
object Player = {40,9,'*'};
object Mob = {40,0,'+'};

void UserInit(){
	Player.x = 40;
	Player.y = 9;
	Mob.x = 40;
	Mob.y = 0;
}

void generateUser(){
	nokia_lcd_set_cursor(Player.x,Player.y);
	nokia_lcd_write_char(Player.sym,1);
}

void generateMob(){
	nokia_lcd_set_cursor(Mob.x,Mob.y);
	nokia_lcd_write_char(Mob.sym,1);
}

// User Input
enum Input_States{INPUT_WAIT,INPUT_LEFT,INPUT_RIGHT};

int Input_Tick(int state){
	switch(state){
		case INPUT_WAIT:
			if(BUTTON1) state = INPUT_RIGHT;
			if(BUTTON2) state = INPUT_LEFT;
			else state = INPUT_WAIT;
			break;
		case INPUT_LEFT:
			state = (BUTTON2)?(INPUT_LEFT):(INPUT_WAIT);
			break;
		case INPUT_RIGHT:
			state = (BUTTON1)?(INPUT_RIGHT):(INPUT_WAIT);
			break;
		default:
			state = INPUT_WAIT;
			break;
	}

	switch(state){
		case INPUT_WAIT:
			break;
		case INPUT_LEFT:
			Player.x += SPEED;
			Mob.x = Player.x;
			break;
		case INPUT_RIGHT:
			Player.x -= SPEED;
			Mob.x = Player.x;
			break;
	}

	return state;
}

// Game Logic
enum Game_States{GAME_WAIT,GAME_ON,GAME_END,GAME_LEVEL_END};

int Game_Tick(int state){
	switch(state){
		case GAME_WAIT:
			state = (BUTTON4)?(GAME_ON):(GAME_WAIT);
			if(state == GAME_ON){
				nokia_lcd_clear();
				UserInit();
				MapInit();
				START = 1;
				END = 0;
			}
			break;
		case GAME_ON:
			state = (BUTTON3)?(GAME_END):(GAME_ON);
			if(state == GAME_END || state == GAME_LEVEL_END){
				nokia_lcd_clear();
				END = 1;
				START = 0;
			}
			break;
		case GAME_END:
			state = GAME_WAIT;
			break;
		case GAME_LEVEL_END:
			state = GAME_WAIT;
			break;
		default:
			state = GAME_WAIT;
	}

	switch(state){
		case GAME_WAIT:
			break;
		case GAME_ON:
			generateUser();
			generateMob();
			break;
		case GAME_END:
			break;
		case GAME_LEVEL_END:
			break;

	}

	return state;
}

int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	LCD_init();
	nokia_lcd_init();
	MapInit();
	TimerOn();
	TimerSet(clockTick);

	task tasks[TASKS_SIZE];

	tasks[0].state = INPUT_WAIT;
	tasks[0].period = 200;
	tasks[0].elapsedTime = tasks[0].period;
	tasks[0].TickFct = &Input_Tick;

	tasks[1].state = GAME_WAIT;
	tasks[1].period = 200;
	tasks[1].elapsedTime = tasks[0].period;
	tasks[1].TickFct = &Game_Tick;

	tasks[2].state = MAP_WAIT;
	tasks[2].period = 200;
	tasks[2].elapsedTime = tasks[0].period;
	tasks[2].TickFct = &Map_Tick;

	while(1){
		PORTD = START | END << 1;

		unsigned char i;
		for (i = 0; i < TASKS_SIZE; i++){
			if(tasks[i].elapsedTime >= tasks[i].period){
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime += clockTick;
		}

		nokia_lcd_render();
		

		while(!TimerFlag);
		TimerFlag = 0;
	}

	return -1;
}
