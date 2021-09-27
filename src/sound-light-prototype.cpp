/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "c:/Users/phela/brave/traffic_light/BraveSensor-Production-Firmware/src/sound-light-prototype.ino"
/*
 * Project sound-light-prototype
 * 
 * Description: Particle Argon firmware for sound & light prototype
 * 
 * Author(s): Blake Shular
 *            Maria Phelan
 */
#include "Particle.h"
#include "consoleFunctionsslp.h"
#include <stdlib.h>

void setup();
void loop();
int change_timer_length(String command);
#line 12 "c:/Users/phela/brave/traffic_light/BraveSensor-Production-Firmware/src/sound-light-prototype.ino"
#define BUZZER D6
#define BUTTON D5

void setup()
{
    Particle.publishVitals(60);
    setupConsoleFunctions();
    pinMode(BUTTON, INPUT);
    pinMode(BUZZER, OUTPUT);
}

void loop()
{
    publish_messages();
}

