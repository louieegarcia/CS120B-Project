#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
#include "io.h"
#include "snes.h"
#include "nokia5110.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
//#define BUTTON1 (~PINA & 0x01)
//#define BUTTON2 (~PINA & 0x02)
//#define BUTTON3 (~PINA & 0x04)
//#define BUTTON4 (~PINA & 0x08)
#define BUTTON1 (SNES_RIGHT)
#define BUTTON2 (SNES_LEFT)
#define BUTTON3 (SNES_START)
#define BUTTON4 (SNES_A)
#define NONE (SNES_NONE)
#define OBSTACLES_SIZE 11
#define TASKS_SIZE 5

// Global variables and flags
unsigned char START = 0;
unsigned char END = 0;
unsigned char SPEED = 1;
unsigned char HEALTH = 3;
unsigned char HIT = 0;
unsigned char MONEY = 0;
unsigned char TIMER = 30;
unsigned short SCORE = 0;
unsigned long clockTick = 100;
unsigned char count = 0;
unsigned short userInput = 0;

// Struct objects
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

// Global Arrays
char nums[10] = {'1','2','3','4','5','6','7','8','9','0'};
unsigned char phone[] = {0x1c,0x16,0x1d,0x01,0x1d,0x16,0x1c,0x00};
object obstacles[OBSTACLES_SIZE]; 
char displayBuffer[32];
char tempBuffer[16];

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
	while(i < OBSTACLES_SIZE-1){
		obstacles[i].sym = '=';
		obstacles[i].x = rand() % 80;
		obstacles[i].y = 42;
		i++;
	}

	obstacles[OBSTACLES_SIZE-1].sym = '$';
	obstacles[OBSTACLES_SIZE-1].x = rand() % 80;
	obstacles[OBSTACLES_SIZE-1].y = 62;
}

void generateMap(){
	unsigned char i = 0;
	nokia_lcd_clear();
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
	Player.y = 10;
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

void writeScore(unsigned char x){
	SCORE += x;
}

void checkHitBox(){
	HIT = 0;
	MONEY = 0;
	unsigned char miss = 0;
	unsigned char i;
	for(i = 0; i < OBSTACLES_SIZE; i++){
		if((Player.x >= obstacles[i].x-4 && Player.x <= obstacles[i].x+4) && Player.y == obstacles[i].y && !HIT){
			if(obstacles[i].sym == '$'){
				MONEY = 1;
				writeScore(200);
			} else{
				HEALTH -= 1;
				nokia_lcd_set_cursor(Mob.x,Mob.y);
				nokia_lcd_write_char(' ',1);
				Mob.y += 2;
				nokia_lcd_set_cursor(Mob.x,Mob.y);
				nokia_lcd_write_char(Mob.sym,1);
				HIT = 1;
			}
		} else if(Player.y == obstacles[i].y){
			if(obstacles[i].sym == '=') miss = 1;
		}
	}

	if(miss && !HIT){
		writeScore(100);
	}
}

// User Input
enum Input_States{INPUT_WAIT,INPUT_LEFT,INPUT_RIGHT};

int Input_Tick(int state){
	switch(state){
		case INPUT_WAIT:
			if(userInput == BUTTON1) state = INPUT_RIGHT;
			else if(userInput == BUTTON2) state = INPUT_LEFT;
			else state = INPUT_WAIT;
			break;
		case INPUT_LEFT:
			state = (userInput == BUTTON2)?(INPUT_LEFT):(INPUT_WAIT);
			break;
		case INPUT_RIGHT:
			state = (userInput == BUTTON1)?(INPUT_RIGHT):(INPUT_WAIT);
			break;
		default:
			state = INPUT_WAIT;
			break;
	}

	switch(state){
		case INPUT_WAIT:
			break;
		case INPUT_LEFT:
			nokia_lcd_clear();
			if(Player.x + SPEED < 80) Player.x += SPEED;
			Mob.x = Player.x;
			break;
		case INPUT_RIGHT:
			nokia_lcd_clear();
			if(Player.x - SPEED >= 0) Player.x -= SPEED;
			Mob.x = Player.x;
			break;
	}

	return state;
}

// Timer
enum Timer_States {TIMER_WAIT,TIMER_DEC};

int Timer_Tick(int state){
	switch(state){
		case TIMER_WAIT:
			state = (START)?(TIMER_DEC):(TIMER_WAIT);
			break;
		case TIMER_DEC:
			state = (!END)?(TIMER_DEC):(TIMER_WAIT);
			break;
		default:
			state = TIMER_WAIT;
			break;
	}

	switch(state){
		case TIMER_DEC:
			if(TIMER > 0) TIMER--;
			break;
	}

	return state;
}

// EEPROM
// uint16_t eeprom_read_word((uint16_t*) addr)
// void eeprom_write_word((uint16_t*) addr, uint16_t value)

void clearEEPROM(){
	eeprom_write_word((uint16_t*)46,0);
}

short getHighScore(){
	return eeprom_read_word((uint16_t*)46);
}

void checkEEPROM(){
	if(SCORE > getHighScore()){
		eeprom_write_word((uint16_t*)46,SCORE);
	}
}

// Timer and Score Display
enum Display_States{DISPLAY_WAIT,DISPLAY_READY,DISPLAY_CALL,DISPLAY_SCORE_TIME,DISPLAY_END,DISPLAY_END2WAIT};

int Display_Tick(int state){
	switch(state){
		case DISPLAY_WAIT:
			state = (userInput == BUTTON4)?(DISPLAY_READY):(DISPLAY_WAIT);
			break;
		case DISPLAY_READY:
			state = (userInput == BUTTON4)?(DISPLAY_CALL):(DISPLAY_READY);
			if(state == DISPLAY_CALL){
				memset(displayBuffer,0,sizeof(displayBuffer));
				memset(tempBuffer,0,sizeof(tempBuffer));
			}
			break;
		case DISPLAY_CALL:
			state = (count == 15)?(DISPLAY_SCORE_TIME):(DISPLAY_CALL);
			if(state == DISPLAY_SCORE_TIME){
				memset(displayBuffer,0,sizeof(displayBuffer));
				memset(tempBuffer,0,sizeof(tempBuffer));
			}
			break;
		case DISPLAY_SCORE_TIME:
			state = (START)?(DISPLAY_SCORE_TIME):(DISPLAY_END);
			if(state == DISPLAY_END){
				memset(displayBuffer,0,sizeof(displayBuffer));
				memset(tempBuffer,0,sizeof(tempBuffer));
			}
			break;
		case DISPLAY_END:
			state = (userInput == BUTTON4)?(DISPLAY_END2WAIT):(DISPLAY_END);
			break;
		case DISPLAY_END2WAIT:
			state = (userInput == NONE)?(DISPLAY_WAIT):(DISPLAY_END2WAIT);
			if(state == DISPLAY_WAIT){
				memset(displayBuffer,0,sizeof(displayBuffer));
				memset(tempBuffer,0,sizeof(tempBuffer));
			}
			break;
		default:
			state = DISPLAY_WAIT;
			break;
	}

	switch(state){
		case DISPLAY_WAIT:
			strcpy(displayBuffer,"Cops and RobbersHigh score: ");
			sprintf(tempBuffer,"%hu",getHighScore());
			strcat(displayBuffer,tempBuffer);
			break;
		case DISPLAY_READY:
			strcpy(displayBuffer,"Ready to play?");
			break;
		case DISPLAY_CALL:
			DisplayScreen(2);
			break;
		case DISPLAY_SCORE_TIME:
			strcpy(displayBuffer,"Score: ");
			sprintf(tempBuffer,"%hu",SCORE);
			strcat(displayBuffer,tempBuffer);
			break;
		case DISPLAY_END:
			strcpy(displayBuffer,"GAME OVER.      SCORE: ");
			sprintf(tempBuffer,"%hu",SCORE);
			strcat(displayBuffer,tempBuffer);
			break;
		case DISPLAY_END2WAIT:
			break;
	}
	return state;
}

void DisplayScreen(unsigned char i){
	if(i == 1){
		LCD_DisplayString(1,"Ready to play?");
	} else if(i == 2){
		LCD_DisplayString(1," 911, there's a robbery!");
		LCD_Cursor(1);
		LCD_WriteData(0x00);
	} else{
		LCD_DisplayString(1,displayBuffer);
	}
}

// Game Logic
enum Game_States{GAME_INIT,GAME_WAIT,GAME_CALL,GAME_ON,GAME_END,GAME_END2INIT,GAME_LEVEL_END};

int Game_Tick(int state){
	
	switch(state){
		case GAME_INIT:
			state = (userInput == BUTTON4)?(GAME_WAIT):(GAME_INIT);
			if(userInput == BUTTON3) clearEEPROM();
			break;
		case GAME_WAIT:
			state = (userInput == BUTTON4)?(GAME_CALL):(GAME_WAIT);
			break;
		case GAME_CALL:
			state = (count == 15)?(GAME_ON):(GAME_CALL);
			if(state == GAME_ON){
				UserInit();
				MapInit();
				START = 1;
				END = 0;
				HEALTH = 3;
				SPEED = 1;
				TIMER = 20;
				SCORE = 0;
				count = 0;
			}
			break;
		case GAME_ON:
			if(!HEALTH){
				state = GAME_END;
			} else if(!TIMER){ 
				state = GAME_LEVEL_END;
				TIMER = 20;
			} else{
				state = GAME_ON;
			}

			if(state == GAME_END){
				END = 1;
				START = 0;
				HIT = 0;
				nokia_lcd_clear();
				checkEEPROM();
			}
			break;
		case GAME_END:
			state = (userInput == BUTTON4)?(GAME_END2INIT):(GAME_END);
			break;
		case GAME_END2INIT:
			state = (userInput == NONE)?(GAME_INIT):(GAME_END2INIT);
			break;
		case GAME_LEVEL_END:
			state = GAME_ON;
			break;
		default:
			state = GAME_WAIT;
			break;
	}

	switch(state){
		case GAME_INIT:
			DisplayScreen(0);
			break;
		case GAME_WAIT:
			DisplayScreen(1);
			break;
		case GAME_CALL:
			count++;
			//DisplayScreen(0);
			break;
		case GAME_ON:
			generateUser();
			generateMob();
			checkHitBox();
			DisplayScreen(0);
			break;
		case GAME_END:
			DisplayScreen(0);
			break;
		case GAME_END2INIT:
			DisplayScreen(0);
			break;
		case GAME_LEVEL_END:
			SPEED *= 2;
			generateUser();
			generateMob();
			checkHitBox();
			DisplayScreen(0);
			break;

	}

	return state;
}

// Main
int main(){
	//DDRA = 0x00; PORTA = 0xFF;
	DDRA = 0x03; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	LCD_init();
	LCD_BuildChar(0,phone);
	nokia_lcd_init();
	MapInit();
	TimerOn();
	TimerSet(clockTick);

	if(getHighScore() == -1) clearEEPROM();

	task tasks[TASKS_SIZE];

	tasks[0].state = INPUT_WAIT;
	tasks[0].period = 200;
	tasks[0].elapsedTime = tasks[0].period;
	tasks[0].TickFct = &Input_Tick;

	tasks[1].state = MAP_WAIT;
	tasks[1].period = 200;
	tasks[1].elapsedTime = tasks[0].period;
	tasks[1].TickFct = &Map_Tick;

	tasks[2].state = GAME_INIT;
	tasks[2].period = 200;
	tasks[2].elapsedTime = tasks[0].period;
	tasks[2].TickFct = &Game_Tick;

	tasks[3].state = TIMER_WAIT;
	tasks[3].period = 1000;
	tasks[3].elapsedTime = 0;
	tasks[3].TickFct = &Timer_Tick;

	tasks[4].state = DISPLAY_WAIT;
	tasks[4].period = 200;
	tasks[4].elapsedTime = tasks[4].period;
	tasks[4].TickFct = &Display_Tick;

	while(1){
		userInput = SNES_Read();
		
		PORTD = (MONEY << 1) | HIT;

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
