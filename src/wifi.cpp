/*
 * Project  updateWifiCredentialsRemotely
 * 
 * Description:  Allows wifi SSID and password to be changed via 
 * a Particle.function() call from the Particle Console
 * 
 * Author: Heidi Fedorak
 * Date:  July 2020
 * 
 * 
 * 
 */
#include "Particle.h"
#include "BraveSensor_firmware_config.h"
#include "wifi.h"

//******************global variable initialization*******************

char mySSIDs[5][MAXLEN] = {CLIENTSSID0, CLIENTSSID1, CLIENTSSID2, CLIENTSSID3, "BraveDiagnostics"};
char myPasswords[5][MAXLEN] = {CLIENTPWD0, CLIENTPWD1, CLIENTPWD2, CLIENTPWD3, "cowardlyarchaiccorp"};

//***********wifi functions***************

//connects to one of 5 stored wifi networks
void connectToWifi(){

  time_t disconnectCounter = Time.now();

  //store the number of times object needs to be written to flash
  int wifiLogCount = readWifiLogFromFlash();
  wifiLogCount++;
  writeWifiLogToFlash(wifiLogCount);

  Log.info("Wifi connection lost, attempting reconnect");
  Log.info("wifiLogCount = %i", wifiLogCount);

  //disconnect from cloud and then turn off wifi module
  Particle.disconnect();
  WiFi.off();
  WiFi.clearCredentials();
  WiFi.on();
  //wait for module to turn on, and for any recently changed network
  //to be able to accept new connections
  delay(15000);

  //set current credentials in temporary memory, specifically in an object of type
  //WiFiCredentials. Look at source code to understand objects as there are 
  //no docs on them.
  for(int i = 0; i < 5; i++){
    time_t timeStarted = Time.now();
    Log.info("Setting credential set: %d", i);
    Log.info(mySSIDs[i]);
    Log.info(myPasswords[i]);


    WiFi.setCredentials(mySSIDs[i], myPasswords[i]);
    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    //wait for wifi to connect or for 15 seconds, whichever is sooner
    waitFor(WiFi.ready, 15000);  

    //wifi.ready() returns true when connected and false when not
    if(WiFi.ready()) {
      Log.info("Connected to wifi.");
      long int connectionLength = Time.now() - timeStarted;
      Log.info("connection process took %ld seconds.\n", connectionLength);
      Log.info("Now wait 10s for cloud connection to re-establish");
      //if we're re-connected, also reconnect to cloud
      Particle.connect();
      //pause for a few moments to allow reconnection
      delay(10000);
      char buffer[1024];
      snprintf(buffer, sizeof(buffer), "{\"Length of disconnect in seconds\":\"%ld\"}", Time.now()-disconnectCounter);
      Particle.publish("Wifi Disconnect Warning",buffer,PRIVATE);
      Log.info(buffer);
      //if we're connected, stop trying credentials
      break;
    } else {
      Log.info("***Failed to connect to wifi***");
      long int connectionLength = Time.now() - timeStarted;
      Log.info("Connection timeout was %ld seconds.\n", connectionLength);
      //else not connected, so continue on to next set of credentials
      continue;
    }

  }

}  //end connectToWifi()

void writeWifiLogToFlash(int wifiLogCount) {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed
  EEPROM.put(ADDR_CONNECT_LOG,wifiLogCount);  

}

int readWifiLogFromFlash() {

  int holder;
  EEPROM.get(ADDR_CONNECT_LOG,holder); 

  return holder;
}

void writeWifiToFlash() {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed
  EEPROM.put(ADDRSSIDS,mySSIDs);  
  EEPROM.put(ADDRPWDS, myPasswords);

}

void readWifiFromFlash() {

  EEPROM.get(ADDRSSIDS,mySSIDs);  
  EEPROM.get(ADDRPWDS,myPasswords);

}


int wifiLog(String logCommand){

  int returnFlag = -1;
  const char* command = logCommand.c_str();

  if(*command == 'e'){
    //if e = echo, function returns the log int
    returnFlag = readWifiLogFromFlash();
  } else if (*command == 'c') {
    //if c = clear, reset the log int to 0
    writeWifiLogToFlash(0);
    returnFlag = readWifiLogFromFlash();
  } else {
    //bad input, return -1
    returnFlag = -1;
  }

  return returnFlag;

}


//change wifi SSID from the cloud

//A cloud function is set up to take one argument of the String datatype. 
//This argument length is limited to a max of 622 characters (since 0.8.0). 
//The String is UTF-8 encoded.
//user should enter a string with format:
//single digit for the index followed by SSID or password, no spaces
//example:  2myNewSSID puts myNewSSID in mySSIDs[2]
int setWifiSSID(String newSSID){

  const char* indexHolder = newSSID.c_str(); 
  
  //compare input to password
  const char* printSSIDs = PASSWORD_FOR_SSIDS;
  int test = strcmp(indexHolder,printSSIDs);
  //if input matches password, print SSIDs to cloud
  if(test == 0){
    readWifiLogFromFlash();
    char holder[640];
	  snprintf(holder, sizeof(holder), "{\"mySSIDs[0]\":\"%s\", \"mySSIDs[1]\":\"%s\", \"mySSIDs[2]\":\"%s\", \"mySSIDs[3]\":\"%s\", \"mySSIDs[4]\":\"%s\", \"connected to:\":\"%s\"}", 
            mySSIDs[0], mySSIDs[1], mySSIDs[2], mySSIDs[3], mySSIDs[4],WiFi.SSID());
    Particle.publish("echo SSIDs", holder, PRIVATE);
  }

  //if input doesn't match password, check first char of string for index digit
  //can't use atoi() because if it fails it returns 0
  //so we don't know if we accidentally entered a string that 
  //starts with a non-integer, or if we've entered a string that
  //starts with 0 on purpose
  //int wifiBufferIndex = atoi(indexHolder);

  //use the ascii table instead, '0' casts to 48, '1' to 49, etc
  //is this a stupid hacky fix or a clever elegant solution? opinions vary...
  int wifiBufferIndex = (int)(*indexHolder) - 48;
  
  //if desired index is out of range, exit with error code -1
  if(wifiBufferIndex < 0 || wifiBufferIndex > 3) return -1;


  //get the rest of the string, skipping 1st character because that is the index
  const char* stringHolder = (newSSID.c_str()+1);

  //copy ssid to correct element of global char array
  strcpy(mySSIDs[wifiBufferIndex], stringHolder);

  //backup in flash memory
  writeWifiToFlash();

  //did it work?
  for(int i = 0; i < 5; i++){
    Log.info("New credential set: %d", i);
    Log.info(mySSIDs[i]);
    Log.info(myPasswords[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}

//user should enter a string with format:
//single digit for the index followed by SSID or password, no spaces
//example:  21password1 puts 1password1 in myPasswords[2]
int setWifiPwd(String newPwd){

  const char* indexHolder = newPwd.c_str(); 

  //compare input to password
  const char* printPwds = PASSWORD_FOR_SSIDS;
  int test = strcmp(indexHolder,printPwds);
  //if input matches password, print SSIDs to cloud
  if(test == 0){
    readWifiLogFromFlash();
    char holder[640];
	  snprintf(holder, sizeof(holder), "{\"myPasswords[0]\":\"%s\", \"myPasswords[1]\":\"%s\", \"myPasswords[2]\":\"%s\", \"myPasswords[3]\":\"%s\", \"myPasswords[4]\":\"%s\", \"connected to:\":\"%s\"}", 
            myPasswords[0], myPasswords[1], myPasswords[2], myPasswords[3], myPasswords[4], WiFi.SSID());
    Particle.publish("echo wifi passwords", holder, PRIVATE);
  }  

  //if input doesn't match password, check first char of string for index digit
  //can't use atoi because if it fails it returns 0
  //so we don't know if we accidentally entered a string that 
  //starts with a non-integer, or if we've entered a string that
  //starts with 0 on purpose
  //int wifiBufferIndex = atoi(indexHolder);

  //use the ascii table instead, '0' casts to 48, '1' to 49, etc
  //is this a stupid hacky fix or a clever elegant solution? opinions vary...
  int wifiBufferIndex = (int)(*indexHolder) - 48;
  
  //if desired index is out of range, exit with error code -1
  if(wifiBufferIndex < 0 || wifiBufferIndex > 3) return -1;

  //get the rest of the string, skipping 1st character because that is the index
  const char* stringHolder = (newPwd.c_str()+1);

  //copy password to correct element of global char array
  strcpy(myPasswords[wifiBufferIndex], stringHolder);

  //backup in flash memory
  writeWifiToFlash();
 
  //did it work?
  for(int i = 0; i < 5; i++){
    Log.info("New credential set: %d", i);
    Log.info(mySSIDs[i]);
    Log.info(myPasswords[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}

void wifiCredsSetup(){

  //read the first two bytes of memory. Particle docs say all
  //bytes of flash initialized to OxF. First two bytes are 0xFFFF
  //on new boards, note 0xFFFF does not correspond to any ASCII chars
  uint16_t checkForContent;
  EEPROM.get(ADDRSSIDS,checkForContent);

  //if memory has not been written to yet, write the original set of 
  //passwords and connect to them.  If memory has been written to 
  //then console functions to update wifi creds have been called before
  //We want to connect to what was written there by the console functions
  //so we read from flash and then connect to those credentials  
  if(checkForContent == 0xFFFF) {
    writeWifiToFlash();
    writeWifiLogToFlash(0);
  } else {
    readWifiFromFlash();
  }

  //if not new Particle, and we want to overwrite flash with original wifi
  //credentials and re-initialize log, we set this define in odetect_config.h
  #if defined(WRITE_ORIGINAL_WIFI)
  writeWifiToFlash();
  writeWifiLogToFlash(0);
  readWifiFromFlash();
  #endif

  #if defined(MANUAL_MODE)
  connectToWifi();
  #endif

}