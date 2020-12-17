#include "Particle.h"
#include "odetect_config.h"
#include "parallelBusReceiver.h"


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

  //load char array to be published to cloud with information that will not change
  //doing this in setup() so sprintf is only called once, to reduce overhead
  
  //Log.info("in checkbus, doorJSON is:");
  //Log.info("%s",doorJSONforCloudPublish);
  unsigned char doorDataHolder;

  if(doorOpen == 1){
    doorDataHolder = 0x02;
    sprintf(doorJSONforCloudPublish, "{\"device\":\"%02X:%02X:%02X\", \"data\":\"%02X\"}", DOORID_BYTE1, DOORID_BYTE2, DOORID_BYTE3,doorDataHolder);
    Log.info("In checkbus, door open if, doorJSON is:");
    Log.info("%s", doorJSONforCloudPublish);
    Particle.publish("IM21 Event", doorJSONforCloudPublish, PRIVATE);
    doorOpen = 0;
  }
  else if(doorClosed == 1){
    doorDataHolder = 0x00;
    sprintf(doorJSONforCloudPublish, "{\"device\":\"%02X:%02X:%02X\", \"data\":\"%02X\"}", DOORID_BYTE1, DOORID_BYTE2, DOORID_BYTE3,doorDataHolder);
    Log.info("In checkbus, door closed if, doorJSON is:");
    Log.info("%s", doorJSONforCloudPublish);
    Particle.publish("IM21 Event", doorJSONforCloudPublish, PRIVATE);
    doorClosed = 0;
  } 
  else if(heartbeat==1){
    doorDataHolder = 0x08;
    sprintf(doorJSONforCloudPublish, "{\"device\":\"%02X:%02X:%02X\", \"data\":\"%02X\"}", DOORID_BYTE1, DOORID_BYTE2, DOORID_BYTE3,doorDataHolder);
    Log.info("In checkbus, door heartbeat if, doorJSON is:");
    Log.info("%s", doorJSONforCloudPublish);
    Particle.publish("IM21 Event", doorJSONforCloudPublish, PRIVATE);
    heartbeat = 0;
  } 
  else if(lowBattery==1){
    doorDataHolder = 0x04;
    sprintf(doorJSONforCloudPublish, "{\"device\":\"%02X:%02X:%02X\", \"data\":\"%02X\"}", DOORID_BYTE1, DOORID_BYTE2, DOORID_BYTE3,doorDataHolder);
    Log.info("In checkbus, door lowbattery if, doorJSON is:");
    Log.info("%s", doorJSONforCloudPublish);
    Particle.publish("IM21 Event", doorJSONforCloudPublish, PRIVATE);
    lowBattery = 0;
  }
  else{
    //Log.info("No signal found");
  }


}