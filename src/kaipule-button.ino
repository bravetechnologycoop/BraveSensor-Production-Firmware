/*
 * Project kaipule-button
 * Description: simple ble code to tell argon to scan for the kaipule button adress and say "button was dinged" if the button is in ble scan results
 * Also publishes data to particle
 * Author: Yousif El-Wishahy
 * Date: 2021-03-23
 */

//global button ble adress (found on label)
const BleAddress buttonAdress = BleAddress("B8:7C:6F:1A:E3:31");

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  BLEScan();
}

void BLEScan(){
  const unsigned int SCAN_RESULT_MAX = 10;
  BleScanResult scanResults[SCAN_RESULT_MAX];
  unsigned char buttonAdvertisingData[BLE_MAX_ADV_DATA_LEN];
  bool condition = false;

  //scan
  int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

  //run through all scanned ble devices until adress found
  for(int i = 0; i < count; i++){
    if (scanResults[i].address() == buttonAdress){
      condition = true;

      //place advertising data in doorAdvertisingData buffer array
      scanResults[i].advertisingData().get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buttonAdvertisingData, BLE_MAX_ADV_DATA_LEN);
      logAndPublishData(buttonAdvertisingData);

      //print out advertising data results over serial
      for (int j = 0; j < sizeof buttonAdvertisingData; j++)
      {
        Serial.printf("%.2x",buttonAdvertisingData[j]);
        Serial.print(" ");
      }
  }
  }
}

void logAndPublishData(unsigned char buttonData[]){
  char publishData[128];
  sprintf(publishData, "{ \"deviceid\": \"%02X %02X %02X\", \"Type ID\": \"%02X\", \"Event Data\": \"%02X\", \"Control Data\": \"%02X\"}", buttonData[1], buttonData[2], buttonData[3], buttonData[4], buttonData[5], buttonData[6]);
  Particle.publish("RB50 Data", publishData, PRIVATE);
  Log.info("published, %02X", buttonData[6]);         
}

