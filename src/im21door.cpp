#include "Particle.h"
#include "CircularBuffer.h"
#include "odetect_config.h"
#include "im21door.h"

//******************global variable initialization*******************


//****************IM21 BLE door sensor functions****************

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
    if(doorAdvertisingData[1] == DOORID_BYTE1 && doorAdvertisingData[2] == DOORID_BYTE2 && doorAdvertisingData[3] == DOORID_BYTE3){

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
        #if defined(SERIAL_DEBUG)
        SerialDebug.println("**********************WE'RE PUBLISHING DOOR DATA***********************");
        SerialDebug.printlnf("Device address: %02X:%02X:%02X:%02X:%02X:%02X",scanResults[ii].address[5], scanResults[ii].address[4], 
                              scanResults[ii].address[3], scanResults[ii].address[2],scanResults[ii].address[1],scanResults[ii].address[0]);
        SerialDebug.printlnf("Advertising data: %02X %02X %02X %02X %02X %02X %02X %02X", doorAdvertisingData[0], doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3], doorAdvertisingData[4], doorAdvertisingData[5], doorAdvertisingData[6], doorAdvertisingData[7]);
        SerialDebug.printlnf("Device ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
        SerialDebug.printlnf("Door Status = %02X", doorAdvertisingData[5]);
        SerialDebug.printlnf("Current, Last Control:  %02X, %02X", currentControl, lastControl);
        SerialDebug.println("past 10 door events are:");
        String doorHistory = String::format("0: %02X, 1: %02X, 2: %02X, 3: %02X, 4: %02X, 5: %02X, 6: %02X, 7: %02X, 8: %02X, 9: %02X",
                                            door[0],door[1],door[2],door[3],door[4],door[5],door[6],door[7],door[8],door[9]);
        SerialDebug.println(doorHistory);
        #endif
      } else if(currentControl == lastControl) {
        //no new door data, we don't publish
        publishDoorData = FALSE;
        doorWarning = FALSE;
        #if defined(SERIAL_DEBUG)
        SerialDebug.printlnf("No new data from door ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
        #endif
      } else {
        //control has incremented by > 1. We are at first event, or have missed an event. Carry on, but publish warning.
        lastControl = currentControl;
        door.unshift(doorAdvertisingData[5]);
        publishDoorData = TRUE;
        doorWarning = TRUE;
        #if defined(SERIAL_DEBUG)
        SerialDebug.printlnf("first event or duplicate event from door ID = %02X %02X %02X",doorAdvertisingData[1], doorAdvertisingData[2], doorAdvertisingData[3]);
        #endif
      }

    if(publishDoorData) {
        String doorData = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }",
                            doorAdvertisingData[1], doorAdvertisingData [2], doorAdvertisingData[3], doorAdvertisingData[5], doorAdvertisingData[6]);
        Particle.publish("Door", doorData, PRIVATE);
        returnFlag = 0;
    }

    if(doorWarning) {
        String doorData = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\", \"warning\": \"First door event after powering on, or missed a door event!\" }",
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