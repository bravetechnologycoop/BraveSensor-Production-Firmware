/*
 * Project wifiTest
 * Description:
 * Author:
 * Date:
 * 
 * Argon from Sampath:
 * Device ID: e00fce68c60d78e0a8e63304
 * Device Secret: KMFNW98HB8K2B38
 * Serial Number: ARNHAB851DXNS46
 * 
 * Argon out of box:
 * Device ID:  e00fce68755e6645225e4a7f
 * 
 * Photon from Sampath:
 * Device ID:  420029000e504b464d323520
 * 
 * 
 */

//*************************defines and global variables that need to be altered during setup*************************

#define USE_SERIAL  //when used, displays serial debugging messages

//#define WRITE_ORIGINALS  //writes original ssid/passwords to flash 

//set initial SSID/password pairs here
char mySSIDs[5][64] = {"Testbed", "Testbed", "ClientSSID1", "ClientSSID2", "BraveDiagnostics"};
char myPasswords[5][64] = {"fireweed5", "fireweed5", "ClientPwd2", "ClientPwd3", "cowardlyarchaiccorp"};


//*************************global macro defines**********************************

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
//0th SSID/password is used during product setup ONLY
//5th SSID/password is Diagnostics network
#define ADDRSSIDS 0
#define ADDRPWDS 320
#define MAXLEN 64

#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB


//*************************function declarations*************************

void blinkLED();
void connectToWifi();
int setWifiSSID(String);
int setWifiPwd(String);   
void writeToFlash();        
void readFromFlash(); 

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
    SerialDebug.println(i);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif

    //WiFi.setCredentials(ssidHolder,pwdHolder);

    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);

    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    
    waitFor(WiFi.ready, 15000);    

    //wifi.ready() returns true when connected and false when not
    if(WiFi.ready()) {
      #if defined(USE_SERIAL)
      SerialDebug.println("Connected to wifi.");
      #endif
      break;
    } else {
      #if defined(USE_SERIAL)
      SerialDebug.println("***Failed to connect to wifi***");
      #endif
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
  writeToFlash();

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
  writeToFlash();
 
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

/*void truncate(char mySSIDs[][MAXLEN]) 
{ 

  //int lenssid = strlen(mySSIDs[0]);


  //char ssidHolder2[lenssid+1];  //for some reason this works in C on my laptop but not on Particle?

  //strcpy(ssidHolder2,mySSIDs[0]);

  }

} 
*/