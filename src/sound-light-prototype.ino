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
#define BUTTON D7
#define TIMEOUT 5000 // in ms

int run();
void buttonPress();
void timerSurpassed();

bool buttonPressed = false;
bool timerPassed = false;
Timer timer(TIMEOUT, timerHandler, true);

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
    if(buttonPressed) {
        buttonPress();
    } else if (timerPassed) {
        timerSurpassed();
    }
}

// cloud function, called by the cloud when an alert is generated, starts alert session
int run(String command) {
    // check if cloud function used right command
    // if (command != "run") {
    //     return -1;
    // }

    // interrupt is attached only when the alert session starts
    attachInterrupt(BUTTON, interruptHandler, RISING);
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

    // then reset the system
    while(1); // placeholder
}

// ends alert session, sends publish message to cloud to escalate
void timerSurpassed() {
    Log.info("Timer surpassed!");
    Particle.publish("timer-surpassed", PRIVATE);
    digitalWrite(BUZZER, LOW);

    // then reset the system
    while(1); // placeholder
}

// callback function to change variable that allows buttonPress to be called
void interruptHandler() {
    buttonPressed = true;
}

// callback function to change variable that allows timerSurpassed to be called
void timerHandler() {
    timerPassed = true;
}
