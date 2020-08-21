/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/heidi/Documents/odetectProductionFirmware/src/odetectProductionFirmware.ino"
/*
 * Project odetectProductionFirmware
 * 
 * Description: Particle Argon/Photon firmware for Brave
 *              ODetect project.
 * 
 * Author(s): Sampath Satti, Wayne Ng, Sajan Rajdev, Heidi Fedorak
 * 
 * Sampath, Wayne Ng, Sajan Rajdev - wrote original XeThru code
 * 
 * Heidi Fedorak - re-wrote XeThru code to be more scalable, and
 *                 and added remote wifi creds update and IM21 
 *                 BLE door sensor features.
 *
 * 
 */

#include "xethru.h"
#include "wifi.h"
#include "im21door.h"

//*************************macros for setup/debugging that need to be altered during setup*************************

//#define PHOTON  //enables code for photon device
//#define MANUAL_MODE  //lets code be flashed via USB serial without a pre-existing wifi connection. Good for debuging.

//*************************System/Startup messages for Particle API***********

void setup();
void loop();
#line 29 "/home/heidi/Documents/odetectProductionFirmware/src/odetectProductionFirmware.ino"
#if defined(MANUAL_MODE)
//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
SYSTEM_THREAD(ENABLED);
//when using manual mode the user code will run immediately when
//the device is powered on
SYSTEM_MODE(MANUAL);
#endif

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif

// setup() runs once, when the device is first turned on.
void setup() {

  //*************global setup******************

  #if defined(USE_SERIAL)
  //start comms with serial terminal for debugging...
  SerialDebug.begin(115200);
  // wait until a character sent from USB host
  waitUntil(SerialDebug.available);
  SerialDebug.println("Key press received, starting code...");
  #endif 

  #if defined(DEBUG_BUILD)
  //mesh and BLE are not compatible with Particle debugger. "Known issue"
    Mesh.off();
    BLE.off();
  #else
    //if we're not debugging, then we need the door sensor to run...
    BLE.on();
  #endif

  xethruSetup();
  //doorSensorSetup() -> consists only of BLE.on, handled above
  wifiCredsSetup();

  //particle console function declarations, belongs in setup() as per docs
  Particle.function("changeSSID", setWifiSSID);  //wifi code
  Particle.function("changePwd", setWifiPwd);    //wifi code
  Particle.function("config", get_configuration_values); //XeThru code

  #if defined(MANUAL_MODE)
  Particle.connect();
  #endif         

  //publish vitals every X seconds
  Particle.publishVitals(120);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  #if defined(USE_SERIAL)
  SerialDebug.print("you're looping");
  #endif

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi();
  }  

  //for every loop check the door data
  checkDoor();
  // For every loop we check to see if we have received any respiration data
  checkXethru();

  delay(1000);

}





