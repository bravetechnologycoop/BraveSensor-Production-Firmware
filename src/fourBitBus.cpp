#include "Particle.h"
#include "odetect_config.h"
#include "fourBitBus.h"


/**********global variables********************/

//not defined as extern so "global" to this file only


//********************setup() functions*************************/
//called from Setup()
void fourBitBusSetup(){

  //configure digial pins of 4 bit bus to output
  pinMode(DOOROPEN_PIN, INPUT_PULLUP);
  pinMode(DOORCLOSED_PIN, INPUT_PULLUP);
  pinMode(HEARTBEAT_PIN, INPUT_PULLUP);
  pinMode(LOWBATTERY_PIN, INPUT_PULLUP);

  attachInterrupt(DOOROPEN_PIN, doorOpenISR, RISING);
  attachInterrupt(DOORCLOSED_PIN, doorClosedISR, RISING);
  attachInterrupt(HEARTBEAT_PIN, heartbeatISR, RISING);
  attachInterrupt(LOWBATTERY_PIN, lowBatteryISR, RISING);

}

//fourBitBusSetup() sub-functions
void doorOpenISR(void){

  doorOpen = 1;

}

void doorClosedISR(void){

  doorClosed = 1;

}

void heartbeatISR(void){

  heartbeat = 1;

}

void lowBatteryISR(void){

  lowBattery = 1;

}



//**********************loop() functions**************************/


