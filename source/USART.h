#include <avr/io.h>
#ifndef USART_H
#define USART_H

#define F_CPU 8000000UL // Assume uC operates at 8MHz
#define BAUD_RATE 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#define UCSRA	UCSR0A
#define UCSRB	UCSR0B
#define UCSRC	UCSR0C
#define UBRRH	UBRR0H
#define UBRRL	UBRR0L
#define UDRE	UDRE0
#define UDR	  UDR0
#define RXC	  RXC0
#define TXC	  TXC0
#define RXEN	RXEN0
#define TXEN	TXEN0

void USART_Init( unsigned int baud ) {
  /* Set baud rate */
  UBRRH = (unsigned char)(baud>>8);
  UBRRL = (unsigned char)baud;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN);
  /* Set frame format: 8data, 2stop bit */
  //UCSRC = (1<<USBS1)|(3<<UCSZ10);
  UCSRC = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Send( unsigned char data ) {
  /* Wait for empty transmit buffer */
  while ( !( UCSRA & (1<<UDRE)) );
  /* Put data into buffer, sends the data */
  UDR = data;
}

void USART_SendString(char *string, unsigned char len){
	unsigned char i;
	while(i < len){
		USART_Send(string[i]);
	}
}

unsigned char USART_Receive( void ) {
  /* Wait for data to be received */
  while ( !(UCSRA & (1<<RXC)) );
  /* Get and return received data from buffer */
  return UDR;
}

void USART_Flush( void ) {
  unsigned char dummy;
  while ( UCSRA & (1<<RXC) ) dummy = UDR;
}

//Functionality - checks if USART is ready to send
//Parameter: None
//Returns: 1 if true else 0
unsigned char USART_IsSendReady()
{
	return (UCSRA & (1 << UDRE));
}

//Functionality - checks if USART has recieved data
//Parameter: None
//Returns: 1 if true else 0
unsigned char USART_HasTransmitted()
{
	return (UCSRA & (1 << TXC));
}

//Functionality - checks if USART has recieved data
//Parameter: None
//Returns: 1 if true else 0
unsigned char USART_HasReceived()
{
	return (UCSRA & (1 << RXC));
}
#endif
