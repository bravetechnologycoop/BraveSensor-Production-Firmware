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
#include "wifi.h"

//******************global variable initialization*******************

char mySSIDs[5][MAXLEN] = {CLIENTSSID0, CLIENTSSID1, CLIENTSSID2, CLIENTSSID3, "BraveDiagnostics"};
char myPasswords[5][MAXLEN] = {CLIENTPWD0, CLIENTPWD1, CLIENTPWD2, CLIENTPWD3, "cowardlyarchaiccorp"};

//***********functions***************

//connects to one of 5 stored wifi networks
void connectToWifi(){

  //turn off wifi module
  WiFi.off();

  //clears credentials from any previously connected networks
  WiFi.clearCredentials();

  //turn on wifi module
  WiFi.on();

  //store these credentials in temporary memory, specifically in an object of type
  //WiFiCredentials. Look at source code to understand objects as there are 
  //no docs on them.
  for(int i = 0; i < 5; i++){

    #if defined(USE_SERIAL)
    SerialDebug.print("Setting credential set: ");
    SerialDebug.println(i+1);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif

    //WiFi.setCredentials(ssidHolder,pwdHolder);

    WiFi.setCredentials(mySSIDs[i], myPasswords[i]);

    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    
    //wait for wifi to connect or for 15 seconds, whichever is sooner
    waitFor(WiFi.ready, 15000);  

    //wifi.ready() returns true when connected and false when not
    if(WiFi.ready()) {
      #if defined(USE_SERIAL)
      SerialDebug.println("Connected to wifi.");
      #endif
      //if we're connected, stop trying credentials
      break;
    } else {
      #if defined(USE_SERIAL)
      SerialDebug.println("***Failed to connect to wifi***");
      #endif
      //else not connected, so continue on to next set of credentials
      continue;
    }

  }

}  //end connectToWifi()


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

//change wifi SSID from the cloud

//A cloud function is set up to take one argument of the String datatype. 
//This argument length is limited to a max of 622 characters (since 0.8.0). 
//The String is UTF-8 encoded.
//user should enter a string with format:
//single digit for the index followed by SSID or password, no spaces
//example:  2myNewSSID puts myNewSSID in mySSIDs[2]
int setWifiSSID(String newSSID){

  const char* indexHolder = newSSID.c_str(); 

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
    #if defined(USE_SERIAL)
    SerialDebug.print("New credential set: ");
    SerialDebug.println(i);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}

//user should enter a string with format:
//single digit for the index followed by SSID or password, no spaces
//example:  21password1 puts 1password1 in myPasswords[2]
int setWifiPwd(String newPwd){

  const char* indexHolder = newPwd.c_str(); 

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
    #if defined(USE_SERIAL)
    SerialDebug.print("New credential set: ");
    SerialDebug.println(i);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}

void wifiCredsSetup(){

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

  #if defined(MANUAL_MODE)
  connectToWifi();
  #endif

}