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
#include <stdlib.h>

void setup();
void loop();
int change_timer_length(String command);
#line 12 "c:/Users/phela/brave/traffic_light/BraveSensor-Production-Firmware/src/sound-light-prototype.ino"
#define BUZZER D6
#define BUTTON D5

int sound_alarm(String command); // cloud function
void button_interrupt();
void timer_overflow();
void publish_messages();

volatile int flag = 0;
volatile int timeout = 5000; // in ms
Timer timer(timeout, timer_overflow, true);

void setup() {
    Particle.publishVitals(60);
    Particle.function("Sound Alarm", sound_alarm);
    Particle.function("Timer Length (integer in ms)", change_timer_length);

    pinMode(BUTTON, INPUT);
    pinMode(BUZZER, OUTPUT);
}

void loop(){
    publish_messages();
}

int change_timer_length(String command){
    //check for non-integer characters
    char command_arr[command.length() + 1];
    strcpy(command_arr, command.c_str());
    for (int i=0; i<command.length(); i++){
        char temp = command_arr[i];
        if (temp!='0' && temp!='1' && temp!='2' && temp!='3' && temp!='4' && temp!='5' && temp!='6' && temp!='7' && temp!='8' && temp!='9'){
            flag = 4;
            return -1;
        }
    }

    timeout = atoi(command); // Update length of escalation timer
    flag = 5;
    return 1;
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

void timer_overflow(){
    detachInterrupt(BUTTON);
    digitalWrite(BUZZER, LOW);
    flag = 2; // escalate response msg
}

void button_interrupt(){
    detachInterrupt(BUTTON);
    digitalWrite(BUZZER, LOW);
    timer.stop();
    flag = 1; // button pressed msg
}

void publish_messages(){
    if(flag == 0){}
    else if(flag == 1){
        Particle.publish("alarm-addressed");
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
    else if(flag == 4){
        Particle.publish("integer-please");
        flag = 0;
    }
    else if(flag == 5){
        Particle.publish("timer-length-updated");
        flag = 0;
    }
} 
