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

void setup();
void loop();
#line 11 "c:/Users/phela/brave/traffic_light/BraveSensor-Production-Firmware/src/sound-light-prototype.ino"
#define BUZZER D6
#define BUTTON D5
#define TIMEOUT 5000 // in ms

int sound_alarm(String command); // cloud function
void button_interrupt();
void timer_overflow();
void publish_messages();

volatile int flag = 0;
Timer timer(TIMEOUT, timer_overflow, true);

void setup() {
    Particle.publishVitals(60);
    Particle.function("Sound Alarm", sound_alarm);

    pinMode(BUTTON, INPUT);
    pinMode(BUZZER, OUTPUT);
}

void loop(){
    publish_messages();
}

int sound_alarm(String command){
    if (command != "run") { // check for correct command
        Particle.publish("wrong-command");
        return -1;
    }

    attachInterrupt(BUTTON, button_interrupt, RISING);
    digitalWrite(BUZZER, HIGH);
    timer.start();
    flag = 3; // alarm sounded msg
    return 1;
}

void button_interrupt(){
    detachInterrupt(BUTTON);
    digitalWrite(BUZZER, LOW);
    timer.stop();
    flag = 1; // button pressed msg
}

void timer_overflow(){
    detachInterrupt(BUTTON);
    digitalWrite(BUZZER, LOW);
    flag = 2; // escalate response msg
}

void publish_messages(){
    if(flag == 0){}
    else if(flag == 1){
        Particle.publish("button-pressed");
        flag = 0;
    }
    else if(flag == 2){
        Particle.publish("escalate-response");
        flag = 0;
    }
    else if(flag == 3){
        Particle.publish("alarm-sounded");
        flag = 0;
    }
} 
