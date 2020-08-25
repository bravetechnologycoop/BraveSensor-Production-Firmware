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
#include "wifi.h"


#if defined(XETHRU_PARTICLE)
#include "xethru.h"
#endif

#if defined(DOOR_PARTICLE)
#include "im21door.h"
#endif

//*************************System/Startup messages for Particle API***********

#if defined(MANUAL_MODE)
//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not interrupted by system/network management functions
SYSTEM_THREAD(ENABLED); 
//when using manual mode the user code will run immediately when the device is powered on
SYSTEM_MODE(MANUAL);
#endif

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif

// setup() runs once, when the device is first turned on.
void setup() {

  //set up serial debugging if set in odetect_config.h file
  #if defined(SERIAL_DEBUG)
    //start comms with serial terminal for debugging...
    SerialDebug.begin(115200);
    // wait until a character sent from USB host
    waitUntil(SerialDebug.available);
    SerialDebug.println("Key press received, starting code...");
  #endif 

  //turn off BLE/mesh if using Particle debug build
  #if defined(DEBUG_BUILD)
  //mesh and BLE are not compatible with Particle debugger. "Known issue"
    Mesh.off();
    BLE.off();
    #if defined(SERIAL_DEBUG)
    SerialDebug.println("**********BLE is OFF*********");
    #endif
  #else
    //if we're not debugging, then we need the door sensor to run...
    BLE.on();
    #if defined(SERIAL_DEBUG)
    SerialDebug.println("**********BLE is ON*********");
    #endif
  #endif

  #if defined(XETHRU_PARTICLE)
    xethruSetup();
  #endif

  //doorSensorSetup() -> consists only of BLE.on, handled above
  wifiCredsSetup();

  //particle console function declarations, belongs in setup() as per docs
  Particle.function("changeSSID", setWifiSSID);  //wifi code
  Particle.function("changePwd", setWifiPwd);    //wifi code

  #if defined(XETHRU_PARTICLE)
  Particle.function("config", get_configuration_values); //XeThru code
  #endif

  //see odetect_config.h for info on manual mode
  #if defined(MANUAL_MODE)
  Particle.connect();
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

  #if defined(SERIAL_DEBUG)
  static int j = 0;
  if (j <= 5) SerialDebug.print("you're looping");
  j++;
  #endif

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi();
  }  

  #if defined(DOOR_PARTICLE)
    //for every loop check for and print door data
    checkDoor();
  #endif

  #if defined(XETHRU_PARTICLE)
  // For every loop we check to see if we have received any respiration data
  checkXethru();
  delay(1000);
  #endif

}





