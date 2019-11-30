#include "mapGeneration.h"
#define BUTTON1 (~PINA & 0x01)
#define BUTTON2 (~PINA & 0x02)

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct) (int);
} task;

unsigned long clockTick = 100;

int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	LCD_init();
	nokia_lcd_init();
	mapInit();
	TimerOn();
	TimerSet(clockTick);

	task tasklist[1];

	tasklist[0].state = MAP_WAIT;
	tasklist[0].period = 500;
	tasklist[0].elapsedTime = tasklist[0].period;
	tasklist[0].TickFct = &Map_Tick;

	while(1){
		START = BUTTON1;
		END = BUTTON2;

		PORTD = START | END;

		unsigned char i = 0;
		for(i = 0; i < sizeof(tasklist); i++){
			if(tasklist[i].elapsedTime >= tasklist[i].period){
				tasklist[i].state = tasklist[i].TickFct(tasklist[i].state);
				tasklist[i].elapsedTime = 0;
			}

			tasklist[i].elapsedTime += clockTick;
		}
		nokia_lcd_set_cursor(10,10);
		nokia_lcd_write_string("hello",1);
		nokia_lcd_render();

		while(!TimerFlag);
		TimerFlag = 0;
	}

	return -1;
}
