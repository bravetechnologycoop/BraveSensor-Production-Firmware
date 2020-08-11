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

#define USE_SERIAL
#define WRITE_ORIGINALS
//#define DEBUG_BUILD

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
//0th SSID/password is used during product setup ONLY
//5th SSID/password is Diagnostics network
#define ADDRSSIDS 0
#define ADDRPWDS 320
#define MAXLEN 64

void blinkLED();
void connectToWifi(char array1[][64], char array2[][64]);
int setWifiSSID(String);
int setWifiPwd(String);   
void writeToFlash();        
void readFromFlash(); 
//char* truncate(char mySSIDs[][MAXLEN]);

//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
SYSTEM_THREAD(ENABLED);
//when using manual mode the user code will run immediately when
//the device is powered on
SYSTEM_MODE(MANUAL);

//STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));


//blue LED built into photon and argon board = D7
int boardLED = D7;
//external LED connected to D4
int redLED = D4;

//setCredentials() does not store credentials in flash memory unless particle has successfully
//connected to that wifi network in the past, so we need to store credentials using EEPROM

char mySSIDs[5][64] = {"wrongssid123456789", "Testbed", "Testbed3", "Testbed4", "Diagnostics"};
char myPasswords[5][64] = {"notright123456789", "fireweed3", "notright3", "notright4", "DiagnosticsPwd"};

//String mySSIDs[5] = {"Testbed", "Testbed", "Testbed", "Testbed", "Diagnostics"};
//String myPasswords[5] = {"fireweed3", "notright2", "notright3", "notright4", "DiagnosticsPwd"};

// setup() runs once, when the device is first turned on.
void setup() {

  #if defined(DEBUG_BUILD)
  Mesh.off();
  BLE.off();
  #endif

  #if defined(USE_SERIAL)
  //start comms with serial terminal for debugging...
  Serial.begin(9600);
  // wait until a character sent from USB host
  waitUntil(Serial.available);

  Serial.println("Key press received, starting code...");
  #endif

  //set blink LED pins to out 
  pinMode(boardLED,OUTPUT); 
  pinMode(redLED,OUTPUT);

  //read the first two bytes of memory. Particle docs say all
  //bytes of flash initialized to OxF. First two bytes are 0xFFFF
  //on new boards, note 0xFFFF does not correspond to any ASCII chars
  #if defined(WRITE_ORIGINALS)
  EEPROM.put(ADDRSSIDS,0xFFFF);
  #endif

  uint16_t checkForContent;
  EEPROM.get(ADDRSSIDS,checkForContent);

  //if memory has not been written to yet, write the initial set of 
  //passwords.  else read what's already in there.
  if(checkForContent == 0xFFFF) {
    writeToFlash();
  } else {
    readFromFlash();
  }

  #if defined(WRITE_ORIGINALS)
  readFromFlash();
  #endif
 
  //loops through 5 different stored networks until connection established
  connectToWifi(mySSIDs,myPasswords);

  //register cloud-connected function BEFORE connecting to cloud
  //these will let me change pwd[0] to fireweed4
  Particle.function("changeSSID", setWifiSSID);
  Particle.function("changePwd", setWifiPwd);

  //connect to cloud
  Particle.connect();            

  //publish vitals every 5 seconds
  //documentation example says this goes in setup, hokay...
  Particle.publishVitals(5);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //has to be called at least once every 20 seconds while in 
  //system manual mode, for... reasons... ???
  //Particle.process();

  #if defined(USE_SERIAL)
  Serial.print("you're looping");
  #endif

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi(mySSIDs, myPasswords);
  }  

  blinkLED();

}

/*void truncate(char mySSIDs[][MAXLEN]) 
{ 

  //int lenssid = strlen(mySSIDs[0]);


  //char ssidHolder2[lenssid+1];  //for some reason this works in C on my laptop but not on Particle?

  //strcpy(ssidHolder2,mySSIDs[0]);

  }

} 
*/

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
    Serial.print("Setting credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    #endif

    //WiFi.setCredentials(ssidHolder,pwdHolder);

    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);

    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    
    waitFor(WiFi.ready, 30000);    

    //wifi.ready() returns true when connected and false when not
    if(WiFi.ready()) {
      #if defined(USE_SERIAL)
      Serial.println("Connected to wifi.");
      #endif
      break;
    } else {
      #if defined(USE_SERIAL)
      Serial.println("***Failed to connect to wifi***");
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

  strcpy(mySSIDs[0],newSSID.c_str());

  //did it work?
  for(int i = 0; i < 5; i++){
    #if defined(USE_SERIAL)
    Serial.print("New credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    #endif
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }


  return -1;

}

int setWifiPwd(String newPwd){

  const char* pwdHolder = newPwd.c_str();

  strcpy(myPasswords[0], pwdHolder);
 
  //did it work?
  for(int i = 0; i < 5; i++){
    #if defined(USE_SERIAL)
    Serial.print("New credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    #endif

    //String holderSSID = String(mySSIDs[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  connectToWifi(mySSIDs,myPasswords);

  //it's never going to return because connectToWifi() above kills connection
  return -1;

}

void blinkLED(){

  //write high to D7
  digitalWrite(boardLED,HIGH);
  digitalWrite(redLED,HIGH);

  //pause
  delay(1000);

  //write low to D7
  digitalWrite(boardLED,LOW);
  digitalWrite(redLED,LOW);
  
  //pause
  delay(250);

}
