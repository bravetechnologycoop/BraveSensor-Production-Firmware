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


#include "firmware_config.h"
#include "xethru.h"
#include "wifi.h"
#include "im21door.h"

//*************************System/Startup messages for Particle API***********

PRODUCT_ID(BRAVE_PRODUCT_ID); //you get this number off the particle console
PRODUCT_VERSION(BRAVE_FIRMWARE_VERSION); //must be an int, so v1.2.01 == 3
SYSTEM_MODE(SEMI_AUTOMATIC);

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif

SerialLogHandler LogHandler(DEBUG_LEVEL);

// setup() runs once, when the device is first turned on.
void setup() {

  //wait three seconds for the log handler to initialize so setup() debug msgs can be printed
  delay(3000);

  #if defined(PHOTON)
  //if we're using a photon that doesn't have BLE, calling BLE will 
  //cause an error.  need to have nothing here so BLE.on or BLE.off
  //are skipped entirely
  #else  
  //if we're not debugging, or a photon, then ble can be on for all other modes
  //BLE must be turned on manually in semi-automatic mode
  BLE.on();
  Log.info("**********BLE is ON*********");
  #endif

  //particle console function declarations, belongs in setup() as per docs
  Particle.function("changeSSID", setSSIDFromConsole);  //wifi code
  Particle.function("changePwd", setPwdFromConsole);    //wifi code
  Particle.function("getWifiLog", getWifiLogFromConsole);       //wifi code

  #if defined(XETHRU_PARTICLE)
  Particle.function("changeXeThruConfigVals", setxeThruConfigValsFromConsole); //XeThru code
  setupXeThru();
  #endif
  #if defined(IM21_PARTICLE)
  Particle.function("changeIM21DoorID",setIM21DoorIDFromConsole);
  setupIM21();
  #endif

  setupWifi();

  //publish vitals every X seconds
  Particle.publishVitals(60);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  static int j = 1;
  if (j <= 1) Log.info("you're looping");
  j++;

  checkWifi();

  //for every loop check the door data
  #if defined(IM21_PARTICLE)
  checkIM21();
  #endif
  // For every loop we check to see if we have received any respiration data
  #if defined(XETHRU_PARTICLE)
  checkXeThru();
  delay(1000);
  #endif

}





