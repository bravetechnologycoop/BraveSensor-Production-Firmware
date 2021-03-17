/*
 * Project odetectProductionFirmware
 * 
 * Description: Particle Argon/Photon firmware for Brave
 *              ODetect project.
 * 
 * Author(s): Sampath Satti, Wayne Ng, Sajan Rajdev, Heidi Fedorak
 * 
 * Sampath Sattie, Wayne Ng, Sajan Rajdev - wrote original XeThru code
 * 
 * Heidi Fedorak - re-wrote XeThru code to be more scalable, and
 *                 and added remote wifi creds update and IM21 
 *                 BLE door sensor features.
 *
 * 
 */
//#include "Particle.h"
#include "xm132.h"

//*************************System/Startup messages for Particle API***********

SerialLogHandler logHandler(LOG_LEVEL_ERROR);

// setup() runs once, when the device is first turned on.
void setup() {

  //publish vitals every X seconds
  Particle.publishVitals(60);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  static bool initialized = false;

  if(!initialized && Particle.connected()){
    xm132Setup();
    initialized = true; 
  }

  if (initialized) {
    checkXM132();
  }


}





