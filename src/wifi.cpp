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
int wifiBufferIndex = -1;
char mySSIDs[5][MAXLEN] = {CLIENTSSID1, CLIENTSSID2, CLIENTSSID3, CLIENTSSID4, "BraveDiagnostics"};
char myPasswords[5][MAXLEN] = {CLIENTPWD1, CLIENTPWD2, CLIENTPWD3, CLIENTPWD4, "cowardlyarchaiccorp"};



//***********functions***************

//connects to one of 5 stored wifi networks
void connectToWifi(char mySSIDs[][MAXLEN], char myPasswords[][MAXLEN]){

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

    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);

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


void writeToFlash() {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed
  EEPROM.put(ADDRSSIDS,mySSIDs);  
  EEPROM.put(ADDRPWDS, myPasswords);

}

void readFromFlash() {

  EEPROM.get(ADDRSSIDS,mySSIDs);  
  EEPROM.get(ADDRPWDS,myPasswords);

}

//change wifi SSID from the cloud

//A cloud function is set up to take one argument of the String datatype. 
//This argument length is limited to a max of 622 characters (since 0.8.0). 
//The String is UTF-8 encoded.
int setWifiSSID(String newSSID){

  //increment mySSIDs and myPasswords array index here
  //user must enter SSID before password or new password
  //will be placed in different array element than SSID
  if(wifiBufferIndex == 3){
    wifiBufferIndex = 0;
  } else{
    wifiBufferIndex++;
  }

  strcpy(mySSIDs[wifiBufferIndex], newSSID.c_str());
  writeToFlash();

  //did it work?
  for(int i = 0; i < 5; i++){
    #if defined(USE_SERIAL)
    SerialDebug.print("New credential set: ");
    SerialDebug.println(i+1);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}

int setWifiPwd(String newPwd){

  strcpy(myPasswords[wifiBufferIndex], newPwd.c_str());
  writeToFlash();
 
  //did it work?
  for(int i = 0; i < 5; i++){
    #if defined(USE_SERIAL)
    SerialDebug.print("New credential set: ");
    SerialDebug.println(i+1);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif

    //String holderSSID = String(mySSIDs[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return wifiBufferIndex;

}