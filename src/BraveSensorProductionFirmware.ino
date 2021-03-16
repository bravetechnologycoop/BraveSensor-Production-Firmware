/*
 * Project BraveSensorProductionFirmware
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

#include "xm132.h"

//*************************System/Startup messages for Particle API***********

SYSTEM_MODE(AUTOMATIC);

SerialLogHandler LogHandler(LOG_LEVEL_INFO);

// setup() runs once, when the device is first turned on.
void setup() {

  //wait three seconds for the log handler to initialize 
  delay(3000);

  //publish vitals every X seconds
  Particle.publishVitals(60);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  static int j = 1;  
  if (j <= 1){
    xm132Setup(); 
    Log.info("you're looping");
  } 
  j++;

  checkXM132();




}





