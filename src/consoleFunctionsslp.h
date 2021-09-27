/*
 * HEADER
 * Console functions for Particle Argon firmware for sound & light prototype
 * based off of 
 * sound-light-prototype by:
 *			  Blake Shular
 *            Maria Phelan
 *
*/

#ifndef CONSOLEFUNCTIONSSLP_H
#define CONSOLEFUNCTIONSSLP_H

//*************************macro defines**********************************


//******************global variable declarations*******************

//*************************function declarations*************************

//setup() functions 

void setupConsoleFunctions();
int change_timer_length(String command);
#line 12 "c:/Users/phela/brave/traffic_light/BraveSensor-Production-Firmware/src/sound-light-prototype.ino"
#define BUZZER D6
#define BUTTON D5

int start_siren(String command); // cloud function
void button_interrupt();
void timer_overflow();
void publish_messages();

int flag = 0;
int period = 5000;
Timer timer(period, timer_overflow, true);