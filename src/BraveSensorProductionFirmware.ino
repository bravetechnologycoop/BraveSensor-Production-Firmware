
#include "Particle.h"
#include "im21door.h"
#include "ins3331.h"
#include "stateMachine.h"
#include "consoleFunctions.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

void setup() {

  // use external antenna on Boron
  BLE.selectAntenna(BleAntennaType::EXTERNAL);
  setupIM21();
  setupINS3331();
  setupConsoleFunctions();
  setupStateMachine();


  Particle.publishVitals(120);  //two minutes
  
}

void loop() {

  //officially sanctioned Mariano (at Particle support) code
  //aka don't send commands to peripherals via UART in setup() because
  //particleOS may not have finished initializing its UART modules
  static bool initialized = false;

  //do once
  if(!initialized && Particle.connected()){ 
    // use external antenna on Boron
    //BLE.selectAntenna(BleAntennaType::EXTERNAL);  
    initializeStateMachineConsts();
    initializeDoorID();
    startINSSerial();
    initialized = true; 
  }

  //do every time loop() is called
  if (initialized) {
    stateHandler();
    getHeartbeat();
  }


}

