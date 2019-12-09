# CS120B-Project
Cops and Robbers is a game where you dodge randomly spawned obstacles while you are being chased by the police (which will be printed on the screen as + symbol). Dodging obstacles gives you 100 points and picking up randomly spawned dollar sign tokens, $, grants you an extra 200 points. If you hit an obstacle, you do not gain any points but you take a point of damage, a max of 3 per game, and the police get that much closer to you. As you progress through the game, every 20 seconds the game speeds up by double. Though technically this game has no end, the difficulty ramps up pretty quickly where the user will reduce their health to 0 in some finite amount of time. At the end, the score is printed to the LCD screen waiting for input to go back to the title screen, where the high score is printed.

# User Guide

| SNES Button | Description |
|-------------|:-----------:|
|LEFT         | Moves the player left during the game |
|RIGHT        | Move the player right during the game |
|A            | Confirms to the next page in the menu |
|START        | Reset EEPROM at title screen|

The purpose of the game is to last as long as you can while dodging obstacles and collecting $ tokens.

# Components/Technologies
The components and technologies used for this project were:
ATMEGA1284
SNES Controller
Nokia 5110 LCD Screen
LEDs and Resistors
1602A LCD Screen
AVRdude

# Complexities
Nokia 5110 LCD
SNES Controller
Game Logic
EEPROM
Special Character on 1602A LCD Screen

# Links To Source
## Nokia 5110: https://github.com/louieegarcia/CS120B-Project/blob/master/source/nokia5110.c
Sourced from https://github.com/LittleBuster/avr-nokia5110
Creates an interface for using the Nokia 5110 LCD Screen.

## Nokia 5110 Charset: https://github.com/louieegarcia/CS120B-Project/blob/master/source/nokia5110_chars.h
Source from https://github.com/LittleBuster/avr-nokia5110/blob/master/nokia5110_chars.h
This creates the data to write to the Nokia 5110 in order to create characters. (a-z, A-Z, !@#$%^&*()~-=+)

## SNES Controller: https://github.com/louieegarcia/CS120B-Project/blob/master/source/snes.h
Sourced from https://github.com/MCoyne1234/CS120B_Labs/blob/master/CS120B_final/CS120B_final/headers/SNES.c
Creates an interface for using the SNES Controller.

## Game Logic and State Machines: https://github.com/louieegarcia/CS120B-Project/blob/master/source/main.c
This is where all the game logic and state machines are implemented. Also the timer and EEPROM was implemented in this file.

# Link To YouTube Video
https://www.youtube.com/watch?v=vnJQltac0Xc&feature=youtu.be
