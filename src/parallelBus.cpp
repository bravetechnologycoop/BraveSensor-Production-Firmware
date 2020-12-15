#include "Particle.h"
#include "odetect_config.h"
#include "parallelBus.h"


/**********global variables********************/

//not defined as extern so "global" to this file only
volatile uint8_t doorOpen = 0;
volatile uint8_t doorClosed = 0;
volatile uint8_t heartbeat = 0;
volatile uint8_t lowBattery = 0;

char doorJSONforCloudPublish[128];


//********************setup() functions*************************/
//called from Setup()
void parallelBusSetup(){

  //configure digial pins of 4 bit bus to output
  pinMode(DOOROPEN_PIN, INPUT_PULLUP);
  pinMode(DOORCLOSED_PIN, INPUT_PULLUP);
  pinMode(HEARTBEAT_PIN, INPUT_PULLUP);
  pinMode(LOWBATTERY_PIN, INPUT_PULLUP);

  attachInterrupt(DOOROPEN_PIN, doorOpenISR, RISING);
  attachInterrupt(DOORCLOSED_PIN, doorClosedISR, RISING);
  attachInterrupt(HEARTBEAT_PIN, heartbeatISR, RISING);
  attachInterrupt(LOWBATTERY_PIN, lowBatteryISR, RISING);
  
  //load char array to be published to cloud with information that will not change
  //doing this in setup() so sprintf is only called once, to reduce overhead
  sprintf(doorJSONforCloudPublish, "{\"device\":\"%02X:%02X:%02X\", \"data\":\"FF\"}", DOORID_BYTE1, DOORID_BYTE2, DOORID_BYTE3);

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

void checkParallelBus(){

  if(doorOpen == 1){
    doorJSONforCloudPublish[30] = 0;
    doorJSONforCloudPublish[31] = 2;
    //Particle.publish("Door",)
    Log.info("%s", doorJSONforCloudPublish);
    doorOpen = 0;
  }
  else if(doorClosed == 1){
    doorJSONforCloudPublish[30] = 0;
    doorJSONforCloudPublish[31] = 0;
    //Particle.publish("Door",)
    Log.info("%s", doorJSONforCloudPublish);
    doorClosed = 0;
  } 
  else if(heartbeat==1){
    doorJSONforCloudPublish[30] = 0;
    doorJSONforCloudPublish[31] = 8;
    //Particle.publish("Door",)
    Log.info("%s", doorJSONforCloudPublish);
    heartbeat = 0;
  } 
  else if(lowBattery==1){
    doorJSONforCloudPublish[30] = 0;
    doorJSONforCloudPublish[31] = 4;
    //Particle.publish("Door",)
    Log.info("%s", doorJSONforCloudPublish);
    lowBattery = 0;
  }
  else{
    Log.info("No signal found");
  }


}