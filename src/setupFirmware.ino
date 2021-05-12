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
#include "flash_addresses.h"


//*************************System/Startup messages for Particle API***********

SYSTEM_MODE(SEMI_AUTOMATIC);

SerialLogHandler LogHandler(DEBUG_LEVEL);

// setup() runs once, when the device is first turned on.
void setup() {

    WiFi.connect();
    Particle.connect();
    Particle.publishVitals(60);

}  //end setup()

// loop() runs over and over again, as quickly as it can execute.
// it is the arduino substitute for while(1) in main()
void loop() {

  static int i = 0;
  if(i < 1){

    //wait 3 seconds for log handler to initialize
    delay(3000);

    //write wifi SSIDs and passwords to flash
    char mySSIDs[5][64] = {CLIENTSSID0, CLIENTSSID1, CLIENTSSID2, CLIENTSSID3, BACKUPSSID};
    char myPasswords[5][64] = {CLIENTPWD0, CLIENTPWD1, CLIENTPWD2, CLIENTPWD3, BACKUPPWD};
    EEPROM.put(ADDR_SSIDS,mySSIDs);  
    EEPROM.put(ADDR_PWDS,myPasswords);

    //write password required to publish ssids or passwords to the cloud
    EEPROM.put(ADDR_PASSWORD_FOR_SSIDS, PASSWORD_FOR_SSIDS);
    EEPROM.put(ADDR_PASSWORD_FOR_PASSWORDS, PASSWORD_FOR_PASSWORDS);

    char pwdforssids[64];
    EEPROM.get(ADDR_PASSWORD_FOR_SSIDS,pwdforssids);
    Log.warn("password for ssids: %s", pwdforssids);

    char pwdforpwds[64];
    EEPROM.get(ADDR_PASSWORD_FOR_PASSWORDS,pwdforpwds);
    Log.warn("password for passwords: %s", pwdforpwds);

    //store the number of times the Argon has connected to wifi 
    int wifiLogCount = -2;
    EEPROM.put(ADDR_WIFI_CONNECT_LOG,wifiLogCount);  

    Log.warn("wrote wifi credentials and disconnect log to flash");

    Particle.publish("Setup Complete", PRIVATE);
    Log.warn("Setup Complete");

    i++;
  }

}



