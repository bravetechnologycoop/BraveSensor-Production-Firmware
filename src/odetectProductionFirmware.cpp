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

//*************************macros for setup/debugging that need to be altered during setup*************************

//#define PHOTON  //enables code for photon device

void setup();
void loop();
#line 25 "/home/heidi/Documents/odetectProductionFirmware/src/odetectProductionFirmware.ino"
#define DOORID_BYTE1 0x96
#define DOORID_BITE2 0x59
#define DOORid_BYTE3 0x27

const size_t SCAN_RESULT_MAX = 30;

BleScanResult scanResults[SCAN_RESULT_MAX];

void checkDoor();

//*************************System/Startup messages for Particle API***********

//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
//SYSTEM_THREAD(ENABLED);
//when using manual mode the user code will run immediately when
//the device is powered on
//SYSTEM_MODE(MANUAL);

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif

//******************global variables************************
//THE ONLY VARIABLES THAT GO HERE SHOULD BE ONES NEEDED BY
//BOTH setup() AND loop() !!!



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
    Mesh.off();
    BLE.off();
  #else
    //if we're not debugging, then we need the door sensor to run...
    BLE.on();
  #endif

  //**********XeThru setup*****************

  pinMode(D6, INPUT_PULLUP); 
  LEDSystemTheme theme; // Enable custom theme
  theme.setColor(LED_SIGNAL_CLOUD_CONNECTED, 0x00000000); // Set LED_SIGNAL_NETWORK_ON to no color
  theme.apply(); // Apply theme settings 
	
  XeThruConfigSettings xethruConfig;

  init_XeThruConfigSettings(&xethruConfig);

  xethru_reset();
  xethru_configuration(&xethruConfig);


  //*********Door sensor setup***********




  //***************Wifi Creds setup*************

  //read the first two bytes of memory. Particle docs say all
  //bytes of flash initialized to OxF. First two bytes are 0xFFFF
  //on new boards, note 0xFFFF does not correspond to any ASCII chars
  #if defined(WRITE_ORIGINALS)
  EEPROM.put(ADDRSSIDS,0xFFFF);
  #endif

  uint16_t checkForContent;
  EEPROM.get(ADDRSSIDS,checkForContent);

  //if memory has not been written to yet, write the original set of 
  //passwords and connect to them.  If memory has been written to 
  //then console functions to update wifi creds have been called before
  //We want to connect to what was written there by the console functions
  //so we read from flash and then connect to those credentials  
  if(checkForContent == 0xFFFF) {
    writeWifiToFlash();
  } else {
    readWifiFromFlash();
  }

  #if defined(WRITE_ORIGINALS)
  readWifiFromFlash();
  #endif

  connectToWifi();
 
  //*******Particle console function declarations*******
  //Must be declared in setup(), but BEFORE connecting to the cloud

  Particle.function("changeSSID", setWifiSSID);  //wifi code
  Particle.function("changePwd", setWifiPwd);    //wifi code
  Particle.function("config", get_configuration_values); //XeThru code

  //connect to cloud
  Particle.connect();            

  //publish vitals every X seconds
  Particle.publishVitals(120);

}  //end setup()


// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  #if defined(USE_SERIAL)
  SerialDebug.println("you're looping");
  #endif

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi();
  }  

  checkDoor();

  // For every loop we check to see if we have received any respiration data
  static int i;
  RespirationMessage msg;
  if(get_respiration_data(&msg)) {
        
    if(msg.state_code != XTS_VAL_RESP_STATE_INITIALIZING || msg.state_code != XTS_VAL_RESP_STATE_ERROR || msg.state_code != XTS_VAL_RESP_STATE_UNKNOWN) {

      // Appends the data received to the end of the array
      if(i%DATA_PER_MESSAGE!=0) {
          strcat(rpm, String(msg.rpm)+" ");
          strcat(distance, String(msg.distance)+" ");
          strcat(breaths, String(msg.breathing_pattern)+" ");
          strcat(slow, String(msg.movement_slow)+" ");
          strcat(fast, String(msg.movement_fast)+" ");
          strcat(x_state, String(msg.state_code)+" ");     
          i++;
      }
      // If the number of data points is enough, append the data without a space and publish the data to the Particle Cloud.
      if(i%DATA_PER_MESSAGE==0) {
          
          strcat(rpm, String(msg.rpm));
          strcat(distance, String(msg.distance));
          strcat(breaths, String(msg.breathing_pattern));
          strcat(slow, String(msg.movement_slow));
          strcat(fast, String(msg.movement_fast));
          strcat(x_state, String(msg.state_code));
          publishData();
          i++;
      } //endif
        
    }//endif

  }//endif

  delay(1000);

}

void checkDoor(){

  // Only scan for 500 milliseconds
  BLE.setScanTimeout(50);
  int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

  //loop over all devices found in the BLE scan
  for (int ii = 0; ii < count; ii++) {

    uint8_t buf[BLE_MAX_ADV_DATA_LEN];

    //place advertising data in a buffer array
    scanResults[ii].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);
    
    //if advertising data contains door sensor's device ID, extract door status and publish it
    if(buf[1] == DOORID_BYTE1 && buf[2] == DOORID_BITE2 && buf[3] == DOORid_BYTE3){

      #if defined(USE_SERIAL)
      SerialDebug.printlnf("Device address: %02X:%02X:%02X:%02X:%02X:%02X",scanResults[ii].address[5], scanResults[ii].address[4], 
                            scanResults[ii].address[3], scanResults[ii].address[2],scanResults[ii].address[1],scanResults[ii].address[0]);
      SerialDebug.printlnf("Advertising data: %02X %02X %02X %02X %02X %02X %02X %02X",buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
      SerialDebug.printlnf("Device ID = %02X %02X %02X",buf[1], buf[2], buf[3]);
      SerialDebug.printlnf("Door Status = %02X", buf[5]);
      SerialDebug.printlnf("Control: %02X", buf[6]);
      #endif

      String data = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }",
                            buf[1], buf [2], buf[3], buf[5], buf[6]);

      Particle.publish("Door", data, PRIVATE);
    } //endif
  }//end for
}



