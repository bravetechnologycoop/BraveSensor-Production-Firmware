#include "Particle.h"
#include "CircularBuffer.h"
#include "odetect_config.h"
#include "im21door.h"


/**********global variables********************/

//not defined as extern so "global" to this file only
IM21DoorIDStruct CurrentDoorID;




/*****************particle console functions********************************/

//particle console function to get/set door sensor ID
int doorSensorIDFromConsole(String command) { // command is a long string with all the config values

  const char* checkForEcho = command.c_str();
  if(*checkForEcho == 'e'){
    IM21DoorIDStruct holder = readDoorIDFromFlash();
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "{\"byte1\":\"%02X\", \"byte2\":\"%02X\", \"byte3\":\"%02X\"}", 
            holder.byte1,holder.byte2,holder.byte3); 
    Particle.publish("Current Door Sensor ID: ",buffer, PRIVATE);
  } else //else we have a command to parse
  {
    const char* byteholder1;
    const char* byteholder2;
    const char* byteholder3;
    int split1 = command.indexOf(',');
    byteholder1 = command.substring(0,split1).c_str();
    CurrentDoorID.byte1 = (uint8_t)strtol(byteholder1,NULL,16);
    int split2 = command.indexOf(',', split1+1);
    byteholder2 = command.substring(split1+1,split2).c_str();
    CurrentDoorID.byte2 = (uint8_t)strtol(byteholder2,NULL,16);
    int split3 = command.indexOf(',', split2+1);
    byteholder3 = command.substring(split2+1,split3).c_str();
    CurrentDoorID.byte3 = (uint8_t)strtol(byteholder3,NULL,16);

    writeDoorIDToFlash(&CurrentDoorID);
  
    //did it get written correctly?
    IM21DoorIDStruct holder = readDoorIDFromFlash();
    Log.info("Contents of flash after console function called:");
    Log.info("byte1: %02X, byte2: %02X, byte3: %02X",holder.byte1,holder.byte2,holder.byte3); 
    //did it get copied to CurrentDoorID correctly?
    Log.info("Door Sensor ID after console function called:");
    Log.info("byte1: %02X, byte2: %02X, byte3: %02X",CurrentDoorID.byte1,CurrentDoorID.byte2,CurrentDoorID.byte3);     

  } //end if-else

  return 1;

}




//********************setup() functions*************************/
//called from Setup()
void doorSensorSetup(){

   //read the first two bytes of memory. Particle docs say all
  //bytes of flash initialized to OxF. First two bytes are 0xFFFF
  //on new boards, note 0xFFFF does not correspond to any ASCII chars

  uint16_t checkForContent;
  EEPROM.get(ADDR_IM21DOORID,checkForContent);
  //if memory has not been written to yet, write the original settings
  //If memory has been written to then console function to update 
  //settings has been called before, so read what was written there
  if(checkForContent == 0xFFFF) {
    initOriginals(&CurrentDoorID);
    writeDoorIDToFlash(&CurrentDoorID);
  } else {
    CurrentDoorID = readDoorIDFromFlash();
  }

  #if defined(WRITE_ORIGINAL_DOORID)
  initOriginals(&CurrentDoorID);
  writeDoorIDToFlash(&CurrentDoorID);
  CurrentDoorID = readDoorIDFromFlash();
  #endif

  Log.info("DoorID at end of setup() is:");
  Log.info("byte1: %02X, byte2: %02X, byte3: %02X",
          CurrentDoorID.byte1,CurrentDoorID.byte2,CurrentDoorID.byte3);


}

//doorSensorSetup() sub-functions
void initOriginals(IM21DoorIDStruct* structToInitialize){

  structToInitialize->byte1 = DOORID_BYTE1;
  structToInitialize->byte2 = DOORID_BYTE2;
  structToInitialize->byte3 = DOORID_BYTE3;

}

void writeDoorIDToFlash(IM21DoorIDStruct* structPtr) {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed
  //passing put() dereferenced pointer to door ID struct
  EEPROM.put(ADDR_IM21DOORID,*structPtr);  

}

IM21DoorIDStruct readDoorIDFromFlash() {

  IM21DoorIDStruct holder;
  EEPROM.get(ADDR_IM21DOORID,holder);
  return holder;  

}



//**********************loop() functions**************************/


//initiates 500ms ble scan for advertising data from door sensor with DOORID specified in odetect_config.h
//if duplicated data is detected, return -2
//if data is not duplicate, store door status in buffer containing 10 most recent door statuses, return 0
//if we reach end of function without having exectued Particle.publish(), return -1
int checkDoor(){

  int returnFlag = -1;
  static const size_t SCAN_RESULT_MAX = 30;
  static BleScanResult scanResults[SCAN_RESULT_MAX];
  static uint8_t doorAdvertisingData[BLE_MAX_ADV_DATA_LEN];
  //keep a history of last 10 door events for debugging purposes
  static CircularBuffer<uint8_t,10> door;
  //cant use CircularBuffer for control bit because all buffers initialize to 0 in 
  //all elements, so you always miss your first event after the device reboots/powers on
  //it's just one variable so easy to implement with flags and if-else
  static uint8_t currentControl = 0x00;
  //lastControl must be initialized as > currentControl+1 or first BLE scan's data will 
  //be published even if it doesn't contain a door event (false positive)
  static uint8_t lastControl = 0xAA;
  static bool publishDoorData;
  static bool doorWarning;
  //OMG WTF is up with the overloading of this scan timeout function??
  //after measuring myself with millis(), 500 = 5 seconds, 50 = 500ms.
  //why is there an order of magnitude difference??  Why is this function
  //not documented in the Particle API or the Arduino API??
  //and who the fuck lists time in units of CENTISECONDS?!? OMG arraarRRRgh
  //setting scan timeout to 50ms = 5 centiseconds
  BLE.setScanTimeout(5);
  int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

  //loop over all devices found in the BLE scan
  for (int ii = 0; ii < count; ii++) {

    //place advertising data in doorAdvertisingData buffer array
    scanResults[ii].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, doorAdvertisingData, BLE_MAX_ADV_DATA_LEN);
    //if advertising data contains door sensor's device ID, extract door status and publish it
    if(doorAdvertisingData[1] == CurrentDoorID.byte1 && doorAdvertisingData[2] == CurrentDoorID.byte2 && doorAdvertisingData[3] == CurrentDoorID.byte3){

      //load the door sensor's current control flag
      currentControl = doorAdvertisingData[6];

      //control flag increments by 1 every time the door data changes status. Publish event if this 
      //occurs, ignore if it doesn't, publish warning if it increments by >1 (missed event)
      if(currentControl == (lastControl+0x01)){
        //control has incremented by 1, great, new event. Store current data and publish.
        lastControl = currentControl;
        door.unshift(doorAdvertisingData[5]);
        publishDoorData = TRUE;
        doorWarning = FALSE;
        //debugging logs
        Log.info("**********************WE'RE PUBLISHING DOOR DATA***********************");
        Log.info("Device address: %02X:%02X:%02X:%02X:%02X:%02X",scanResults[ii].address[5], scanResults[ii].address[4], 
                              scanResults[ii].address[3], scanResults[ii].address[2],scanResults[ii].address[1],scanResults[ii].address[0]);
        Log.info("Advertising data: %02X %02X %02X %02X %02X %02X %02X %02X", doorAdvertisingData[0], doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3], doorAdvertisingData[4], doorAdvertisingData[5], doorAdvertisingData[6], doorAdvertisingData[7]);
        Log.info("Device ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
        Log.info("Door Status = %02X", doorAdvertisingData[5]);
        Log.info("Current, Last Control:  %02X, %02X", currentControl, lastControl);
        Log.info("past 10 door events are:");
        String doorHistory = String::format("0: %02X, 1: %02X, 2: %02X, 3: %02X, 4: %02X, 5: %02X, 6: %02X, 7: %02X, 8: %02X, 9: %02X",
                                            door[0],door[1],door[2],door[3],door[4],door[5],door[6],door[7],door[8],door[9]);
        Log.info(doorHistory);
        //end debugging logs
      } else if(currentControl == lastControl) {
        //no new door data, we don't publish
        publishDoorData = FALSE;
        doorWarning = FALSE;
        Log.info("No new data from door ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
      } else {
        //control has incremented by > 1. We are at first event, or have missed an event. Carry on, but publish warning.
        lastControl = currentControl;
        door.unshift(doorAdvertisingData[5]);
        publishDoorData = TRUE;
        doorWarning = TRUE;
        Log.info("first event or duplicate event from door ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
      }

    if(publishDoorData) {
        String doorData = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }",
                            doorAdvertisingData[1], doorAdvertisingData [2], doorAdvertisingData[3], doorAdvertisingData[5], doorAdvertisingData[6]);
        Particle.publish("IM21", doorData, PRIVATE);
        returnFlag = 0;
    }

    if(doorWarning) {
        String doorData = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\", \"warning\": \"Missed a door event, OR new doorID, OR Particle was rebooted.\" }",
                            doorAdvertisingData[1], doorAdvertisingData [2], doorAdvertisingData[3], doorAdvertisingData[5], doorAdvertisingData[6]);
        Particle.publish("Door Warning", doorData, PRIVATE);
        returnFlag = 0;
    }

    } //endif that weeds out door's signal from other BLE signals

  }//end for loop that searches through all BLE signals

//if we reach here and haven't published, returnFlag = -1
//if we have published, it will have been set to returnFlag = 0 above
return returnFlag;

}