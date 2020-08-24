#include "Particle.h"
#include "odetect_config.h"
#include "im21door.h"

//******************global variable initialization*******************

//these variables only need to be "global" to door code
//not entire program, so they are not defined as externs in header file
const size_t SCAN_RESULT_MAX = 30;
BleScanResult scanResults[SCAN_RESULT_MAX];

//****************IM21 BLE door sensor functions****************

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

      #if defined(SERIAL_DEBUG)
      SerialDebug.printlnf("Device address: %02X:%02X:%02X:%02X:%02X:%02X",scanResults[ii].address[5], scanResults[ii].address[4], 
                            scanResults[ii].address[3], scanResults[ii].address[2],scanResults[ii].address[1],scanResults[ii].address[0]);
      SerialDebug.printlnf("Advertising data: %02X %02X %02X %02X %02X %02X %02X %02X",buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
      SerialDebug.printlnf("Device ID = %02X %02X %02X",buf[1], buf[2], buf[3]);
      SerialDebug.printlnf("Door Status = %02X", buf[5]);
      SerialDebug.printlnf("Control: %02X", buf[6]);
      #endif

      String data = String::format("{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }",
                            buf[1], buf [2], buf[3], buf[5], buf[6]);

      //Particle.publish("Door", data, PRIVATE);

    } //endif
  }//end for
}