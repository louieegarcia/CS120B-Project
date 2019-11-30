#include "global.h"

enum mapStates {MAP_WAIT,MAP_GENERATE};

object obstacles[10]; 

void mapInit(){
	unsigned char i = 0;
	for(i = 0; i<sizeof(obstacles); i++){
		obstacles[i].sym = 'a';
		obstacles[i].x = 10;
		obstacles[i].y = 10;
	}
}

void generateMap(){
	unsigned char i = 0;
	for(i = 0; i<sizeof(obstacles); i++){
		obstacles[i].y += 1;
		nokia_lcd_set_cursor(obstacles[i].x,obstacles[i].y);
		nokia_lcd_write_char(obstacles[i].sym,1);
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
