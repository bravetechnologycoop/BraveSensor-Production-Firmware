/*
 * Project sound-light-prototype
 * 
 * Description: Particle Argon firmware for sound & light prototype
 * 
 * Author(s): Blake Shular
 * 
 */

#include "Particle.h"

#define BUZZER D6
#define BUTTON D8
#define TIMEOUT 10000 // in ms
// #define TONE 1760 // in Hz

unsigned int timeout = 10000;

int run();
void buttonPress();
void timerSurpassed();
bool buttonPressed = false;

SerialLogHandler logHandler(LOG_LEVEL_INFO);

void setup() {
    Particle.publishVitals(60);

    pinMode(BUZZER, OUTPUT);
    pinMode(BUTTON, INPUT);

    delay(5000);
    Log.info("Setup!");
    run();
}

void loop() {
    if(buttonPressed){
        buttonPress();
    }
}

// called by the cloud when an alert is generated, starts alert session
int run() {
    // interrupt is attached only when the alert session starts
    attachInterrupt(BUTTON, interruptHandler, RISING);
    Timer timer(timeout, timerSurpassed, true);
    timer.start();
    digitalWrite(BUZZER, HIGH);
    Log.info("Running!");
    loop();
    return 1;
}

// ends alert session, sends publish message to cloud
void buttonPress() {
    Log.info("Button pressed!");
    Particle.publish("button-pressed", PRIVATE);
    digitalWrite(BUZZER, LOW);
}

void timerSurpassed() {
    Log.info("Timer surpassed!");
    Particle.publish("timer-surpassed", PRIVATE);
    digitalWrite(BUZZER, LOW);
}

void interruptHandler(){
    buttonPressed = true;
}
