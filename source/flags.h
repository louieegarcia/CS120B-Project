#include <avr/interrupt.h>
#include <avr/io.h>
#include "io.h"
#include "timer.h"
#include "USART.h"
#include "nokia5110.h"

unsigned char START = 0;
unsigned char END = 0;

typedef object{
	unsigned char x;
	unsigned char y;
	unsigned char sym;
} obj;

