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

#include "setupFirmware.h"

//**********FLASH ADDRESSES***********************
//wifi
#define ADDR_SSIDS 0   		  		    	//sizeof = 320
#define ADDR_PWDS 320			        		//sizeof = 320
#define ADDR_WIFI_CONNECT_LOG 640			//sizeof = 4

//xethru
#define ADDR_XETHRU_LED 644		      	//sizeof = 4
#define ADDR_XETHRU_NOISEMAP 648      //sizeof = 4
#define ADDR_XETHRU_SENSITIVITY 652   //sizeof = 4
#define ADDR_XETHRU_MIN_DETECT 656    //sizeof = 4
#define ADDR_XETHRU_MAX_DETECT 660    //sizeof = 4

//im21 door sensor
#define ADDR_IM21_DOORID 664		    	//sizeof = 3

//general device settings
#define ADDR_LOCATION_ID 667          //sizeof = 64
#define ADDR_DEVICE_ID 731            //sizeof = 4
#define ADDR_DEVICE_TYPE 735          //sizeof = 64

//next available memory location is 735+64 = 799


//*************************System/Startup messages for Particle API***********

SerialLogHandler LogHandler(DEBUG_LEVEL);

// setup() runs once, when the device is first turned on.
void setup() {

  //write wifi SSIDs and passwords to flash
  char mySSIDs[5][64] = {CLIENTSSID0, CLIENTSSID1, CLIENTSSID2, CLIENTSSID3, "thisisastringwith63characterslalalalalalalalalalalalalalalala63"};
  char myPasswords[5][64] = {CLIENTPWD0, CLIENTPWD1, CLIENTPWD2, CLIENTPWD3, "cowardlyarchaiccorp"};
  EEPROM.put(ADDR_SSIDS,mySSIDs);  
  EEPROM.put(ADDR_PWDS,myPasswords);

  //store the number of times the Argon has connected to wifi 
  int wifiLogCount = 0;
  EEPROM.put(ADDR_WIFI_CONNECT_LOG,wifiLogCount);  

  Log.warn("wrote wifi credentials to flash");

  //write general device settings to flash
  char locationID[64] = LOCATIONID;
  char deviceType[64] = DEVICETYPE;
  int deviceID = DEVICEID;

  EEPROM.put(ADDR_LOCATION_ID, locationID);
  EEPROM.put(ADDR_DEVICE_TYPE, deviceType);
  EEPROM.put(ADDR_DEVICE_ID, deviceID);

  Log.warn("wrote general device settings to flash");

  #if defined(XETHRU_PARTICLE)
  //load settings into variable of the appropriate type instead of trusting
  //EEPROM.put constructor to get it right. If I want min_detect to be a 
  //4 byte float and put() types it as an 8 byte double, it will over-write
  //other areas of memory meant for other variables
  int led = XETHRU_LED_SETTING; 
  int noisemap = XETHRU_NOISEMAP_SETTING;
  int sensitivity = XETHRU_SENSITIVITY_SETTING;
  float min_detect = XETHRU_MIN_DETECT_SETTING;
  float max_detect = XETHRU_MAX_DETECT_SETTING;

  EEPROM.put(ADDR_XETHRU_LED, led);  
  EEPROM.put(ADDR_XETHRU_NOISEMAP, noisemap);  
  EEPROM.put(ADDR_XETHRU_SENSITIVITY, sensitivity);  
  EEPROM.put(ADDR_XETHRU_MIN_DETECT, min_detect);  
  EEPROM.put(ADDR_XETHRU_MAX_DETECT, max_detect);  

  Log.warn("Wrote XeThru values to flash");
  #endif

  #if defined(IM21_PARTICLE)

  unsigned char doorbyte1 = 0xAA;
  unsigned char doorbyte2 = 0xAA;
  unsigned char doorbyte3 = 0xAA;

  EEPROM.put(ADDR_IM21_DOORID, doorbyte1);
  EEPROM.put((ADDR_IM21_DOORID+1), doorbyte2);
  EEPROM.put((ADDR_IM21_DOORID+2), doorbyte3);

  Log.warn("wrote IM21 door ID to flash");

  #endif

  Log.warn("Setup Complete");



}  //end setup()

// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

}



