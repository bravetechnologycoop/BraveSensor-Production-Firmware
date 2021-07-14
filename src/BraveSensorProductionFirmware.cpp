/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/Seto/Documents/Brave/BraveSensor-Production-Firmware/src/BraveSensorProductionFirmware.ino"
/*
 * Brave firmware state machine for single Boron
 * written by Heidi Fedorak, Apr 2021
 * 
 * Edits for Xethru local state machine by James Seto
*/

#include "Particle.h"
#include "im21door.h"
#include "stateMachine.h"
#include "consoleFunctions.h"
#include "wifi.h"
#include "xethru.h"

void setup();
void loop();
#line 15 "/Users/Seto/Documents/Brave/BraveSensor-Production-Firmware/src/BraveSensorProductionFirmware.ino"
#define DEBUG_LEVEL LOG_LEVEL_INFO
#define BRAVE_FIRMWARE_VERSION 2002 //see versioning notes in the readme
#define BRAVE_PRODUCT_ID 12858 //12858 = beta units, 12876 = production units

PRODUCT_ID(BRAVE_PRODUCT_ID); //you get this number off the particle console, see readme for instructions
PRODUCT_VERSION(BRAVE_FIRMWARE_VERSION); //must be an int, see versioning notes above
SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(DEBUG_LEVEL);

void setup() {

  // use external antenna on Boron
  BLE.selectAntenna(BleAntennaType::EXTERNAL);
  setupIM21();
  setupXeThru();
  //Log.info("Xethru Setup Complete --------------------------");
  setupConsoleFunctions();
  setupStateMachine();
  setupWifi();
  //Log.info("All Setup Complete -----------------------------");
  



  Particle.publishVitals(120);  //two minutes
  
}

void loop() {

    checkWifi();

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
    initialized = true; 
  }

  //do every time loop() is called
  if (initialized) {
    stateHandler();
    getHeartbeat();
  }


}

