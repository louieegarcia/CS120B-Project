#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
#include "io.h"
#include "timer.h"
//#include "USART.h"
#include "nokia5110.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char START = 0;
unsigned char END = 0;
unsigned char SPEED = 1;

typedef struct object{
	unsigned char x;
	unsigned char y;
	unsigned char sym;
} object;

