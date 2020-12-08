/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/heidi/Programming/particleProgramming/odetectProductionFirmware/src/odetectProductionFirmware.ino"
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

#include "odetect_config.h"
#include "xethru.h"
#include "wifi.h"
#include "im21door.h"
#include "xm132.h"

//*************************System/Startup messages for Particle API***********

void setup();
void loop();
#line 26 "/home/heidi/Programming/particleProgramming/odetectProductionFirmware/src/odetectProductionFirmware.ino"
SerialLogHandler logHandler(DEBUG_LEVEL);

#if defined(MANUAL_MODE)
//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not interrupted by system/network management functions
SYSTEM_THREAD(ENABLED); 
//when using manual mode the user code will run immediately when the device is powered on
SYSTEM_MODE(MANUAL);
#else
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif

// setup() runs once, when the device is first turned on.
void setup() {

  #if defined(PHOTON)
  //if we're using a photon that doesn't have BLE, calling BLE will 
  //cause an error.  need to have nothing here so BLE.on or BLE.off
  //are skipped entirely
  #else  
  //if we're not debugging, or a photon, then ble can be on for all other modes
  BLE.on();
  Log.info("**********BLE is ON*********");
  #endif

  //particle console function declarations, belongs in setup() as per docs
  //in manual or semi-automatic mode, these must be declared before Particle.connect() is called
  Particle.function("changeSSID", setWifiSSID);  //wifi code
  Particle.function("changePwd", setWifiPwd);    //wifi code
  Particle.function("getWifiog", wifiLog);       //wifi code

  wifiCredsSetup();

  #if defined(XETHRU_PARTICLE)
  Particle.function("xethruConfigVals", xethruConfigValesFromConsole); //XeThru code
  xethruSetup();
  #endif
  #if defined(DOOR_PARTICLE)
  Particle.function("doorSensorID",doorSensorIDFromConsole);
  doorSensorSetup();
  #endif
  #if defined(XM132_PARTICLE)
  Log.info("I'm in setup, about to enter xm132setup()");

  #endif

  //see odetect_config.h for info on manual mode
  #if defined(MANUAL_MODE)
  Particle.connect();
  Particle.process();
  #endif         

  //publish vitals every X seconds
  Particle.publishVitals(60);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  //see odetect_config.h for info on manual mode
  #if defined(MANUAL_MODE)
  Particle.process();
  #endif    

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi();
  }  


  static int j = 1;
  static int numberOfLoops = 1;
  if (j <= numberOfLoops) {
    Log.info("you're looping");
    xm132Setup();
  }
  j++;

  //for every loop check the door data
  #if defined(DOOR_PARTICLE)
  checkDoor();
  #endif
  // For every loop we check to see if we have received any respiration data
  #if defined(XETHRU_PARTICLE)
  checkXethru();
  delay(1000);
  #endif
  #if defined(XM132_PARTICLE)
  
  #endif



}





