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
char mySSIDs[5][64] = {"ClientSSID1", "Testbed", "ClientSSID1", "ClientSSID2", "BraveDiagnostics"};
char myPasswords[5][64] = {"ClientPwd1", "fireweed5", "ClientPwd2", "ClientPwd3", "cowardlyarchaiccorp"};


//*************************global macro defines**********************************

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
//0th SSID/password is used during product setup ONLY
//5th SSID/password is Diagnostics network
#define ADDRSSIDS 0
#define ADDRPWDS 320
#define MAXLEN 64

#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB


//*************************global variable defines*******************
int addr = -1;


//*************************function declarations*************************

void blinkLED();
void connectToWifi(char array1[][64], char array2[][64]);
int setWifiSSID(String);
int setWifiPwd(String);   
void writeToFlash();        
void readFromFlash(); 
//char* truncate(char mySSIDs[][MAXLEN]);


//connects to one of 5 stored wifi networks
void connectToWifi(char mySSIDs[][64], char myPasswords[][64]){

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
int setWifiSSID(String newSSID){

  //increment mySSIDs and myPasswords array index here
  //user must enter SSID before password or new password
  //will be placed in different array element than SSID
  if(addr == 3){
    addr = 0;
  } else{
    addr++;
  }

  strcpy(mySSIDs[addr], newSSID.c_str());
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

  return addr;

}

int setWifiPwd(String newPwd){

  strcpy(myPasswords[addr], newPwd.c_str());
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

  return addr;

}

/*void truncate(char mySSIDs[][MAXLEN]) 
{ 

  //int lenssid = strlen(mySSIDs[0]);


  //char ssidHolder2[lenssid+1];  //for some reason this works in C on my laptop but not on Particle?

  //strcpy(ssidHolder2,mySSIDs[0]);

  }

} 
*/