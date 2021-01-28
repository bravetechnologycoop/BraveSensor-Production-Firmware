#include "Particle.h"
#include "firmware_config.h"
#include "flash_addresses.h"
#include "im21door.h"


/**********global variables********************/

//not defined as extern so "global" to this file only
//needs to be global because it is used in setup(), loop(), and console function
IM21DoorID globalDoorID;


/*****************************************************************console functions***********************************************************************/

//particle console function to get/set door sensor ID
int setIM21DoorIDFromConsole(String command) { // command is a long string with all the config values

  //get pointer to user-entered string
  const char* checkForEcho = command.c_str();

  //if echo, publish current door ID
  if(*checkForEcho == 'e'){

    IM21DoorID holder = readIM21DoorIDFromFlash();

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "{\"byte1\":\"%02X\", \"byte2\":\"%02X\", \"byte3\":\"%02X\"}", 
            holder.byte1,holder.byte2,holder.byte3); 
    Particle.publish("Current Door Sensor ID: ",buffer, PRIVATE);
  } 
  else //else not echo, so we have a new door ID to parse
  {
    //parse input string and update global door ID 
    const char* byteholder1;
    const char* byteholder2;
    const char* byteholder3;
    int split1 = command.indexOf(',');
    byteholder1 = command.substring(0,split1).c_str();
    globalDoorID.byte3 = (uint8_t)strtol(byteholder1,NULL,16);
    int split2 = command.indexOf(',', split1+1);
    byteholder2 = command.substring(split1+1,split2).c_str();
    globalDoorID.byte2 = (uint8_t)strtol(byteholder2,NULL,16);
    int split3 = command.indexOf(',', split2+1);
    byteholder3 = command.substring(split2+1,split3).c_str();
    globalDoorID.byte1 = (uint8_t)strtol(byteholder3,NULL,16);

    //write new global door ID to flash
    writeIM21DoorIDToFlash(globalDoorID);
  
    //did it get written correctly?
    IM21DoorID check = readIM21DoorIDFromFlash();

    Log.warn("Door ID in flash after console function called:");
    Log.warn("byte1: %02X, byte2: %02X, byte3: %02X",check.byte1,check.byte2,check.byte3); 

  } //end if-else

  return 1;

}

void writeIM21DoorIDToFlash(IM21DoorID doorID) {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed

  EEPROM.put(ADDR_IM21_DOORID,doorID.byte1);  
  EEPROM.put((ADDR_IM21_DOORID+1),doorID.byte2);  
  EEPROM.put((ADDR_IM21_DOORID+2),doorID.byte3);  

}



//********************setup() functions*************************/
//called from Setup()
void setupIM21(){

  //load global door ID from flash. Loading it once here instead of re-reading it from flash
  //every time loop() is called.  This forces the door ID to be global, but also saves on 
  //reading from flash

  printDeviceIdentifiersFromFlash();  

  globalDoorID = readIM21DoorIDFromFlash();

  Log.warn("DoorID read from flash during setup() is:");
  Log.warn("byte1: %02X, byte2: %02X, byte3: %02X",
          globalDoorID.byte1,globalDoorID.byte2,globalDoorID.byte3);

}


void printDeviceIdentifiersFromFlash(){

  char locationID[MAXLEN];
  int deviceID;
  char deviceType[MAXLEN];

  EEPROM.get(ADDR_LOCATION_ID, locationID);
  EEPROM.get(ADDR_DEVICE_TYPE, deviceType);
  EEPROM.get(ADDR_DEVICE_ID, deviceID);

  Log.warn("Device Identifiers read from flash during IM21 setup:");
  Log.warn("location ID: %s, device ID: %d, deviceType: %s", locationID, deviceID, deviceType); 

}



IM21DoorID readIM21DoorIDFromFlash() {

  IM21DoorID holder;

  EEPROM.get(ADDR_IM21_DOORID,holder.byte1);  
  EEPROM.get((ADDR_IM21_DOORID+1),holder.byte2);  
  EEPROM.get((ADDR_IM21_DOORID+2),holder.byte3);  
  
  return holder;  

}


//******************************************************************loop() functions************************************************************************/


//initiates 50ms ble scan for advertising data from door sensor with DOORID specified in flash memory
void checkIM21(){

  static int initialDoorDataFlag = 1;
  static doorData previousDoorData = {0x00, 0x00};
  static doorData currentDoorData = {0x00, 0x00};

  const unsigned int SCAN_RESULT_MAX = 10;
  BleScanResult scanResults[SCAN_RESULT_MAX];
  unsigned char doorAdvertisingData[BLE_MAX_ADV_DATA_LEN]; 

  //OMG WTF is up with the overloading of this scan timeout function??
  //used millis() to test/measure, timeout(1) = 13-14 ms. timout(5) = 53-54ms
  //who the fuck uses time in units of CENTISECONDS??  omg arraarRRRgh
  //setting scan timeout (how long scan runs for) to 50ms = 5 centiseconds
  BLE.setScanTimeout(5);
  int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

  //loop over all devices found in the BLE scan
  for (int i = 0; i < count; i++) {

    //place advertising data in doorAdvertisingData buffer array
    scanResults[i].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, doorAdvertisingData, BLE_MAX_ADV_DATA_LEN);

    //if advertising data contains door sensor's device ID, extract door status and dump it in the queue
    if(doorAdvertisingData[1] == globalDoorID.byte1 && doorAdvertisingData[2] == globalDoorID.byte2 && doorAdvertisingData[3] == globalDoorID.byte3){

      currentDoorData.doorStatus = doorAdvertisingData[5];
      currentDoorData.controlByte = doorAdvertisingData[6];

      Log.info("raw door sensor output - control:  prev, current: 0x%02X, 0x%02X", previousDoorData.controlByte, currentDoorData.controlByte);
      Log.info("raw door sensor output - data byte prev, current: 0x%02X, 0x%02X", previousDoorData.doorStatus, currentDoorData.doorStatus);

      //if this is the first door event received after firmware bootup, publish
      if(initialDoorDataFlag){

        initialDoorDataFlag = 0;
        logAndPublishDoorData(previousDoorData,currentDoorData);
        previousDoorData = currentDoorData;   
      }
      //if this curr = prev + 1, all is well, publish
      else if(currentDoorData.controlByte == (previousDoorData.controlByte+0x01)){

        logAndPublishDoorData(previousDoorData,currentDoorData);
        previousDoorData = currentDoorData;  
      }
      //if curr > prev + 1, missed an event, publish warning
      else if(currentDoorData.controlByte > (previousDoorData.controlByte+0x01)){

        Log.error("curr > prev + 1, WARNING WARNING WARNING, missed door event!");
        logAndPublishDoorWarning(previousDoorData,currentDoorData);
        previousDoorData = currentDoorData;  
      }
      //special case for when control byte rolls over from FF to 00, don't want to publish missed door event warning
      else if ((currentDoorData.controlByte == 0x00) && (previousDoorData.controlByte == 0xFF)){

        logAndPublishDoorData(previousDoorData,currentDoorData);
        previousDoorData = currentDoorData;  

      }
      else {
        //no new data, do nothing
        Log.info("no new data");

      } //end publish-if-else

    }//end door sensor identifier if

  }//endfor

} //end checkIM21()

void logAndPublishDoorData(doorData previousDoorData, doorData currentDoorData){

  char doorPublishBuffer[128];

  sprintf(doorPublishBuffer, "{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }", 
          globalDoorID.byte1, globalDoorID.byte2, globalDoorID.byte3, currentDoorData.doorStatus, currentDoorData.controlByte);
  Particle.publish("IM21 Data", doorPublishBuffer, PRIVATE);
  Log.info("published");

}

void logAndPublishDoorWarning(doorData previousDoorData, doorData currentDoorData){

  char doorPublishBuffer[128];

  sprintf(doorPublishBuffer, "{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\", \"warning\": \"Missed a door event!\" }", 
          globalDoorID.byte1, globalDoorID.byte2, globalDoorID.byte3, currentDoorData.doorStatus, currentDoorData.controlByte);
  Particle.publish("IM21 Warning", doorPublishBuffer, PRIVATE);
  Log.info("published");

}