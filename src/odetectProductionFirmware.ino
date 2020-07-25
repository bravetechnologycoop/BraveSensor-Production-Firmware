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


void printWifiCreds();
void blinkLED();
void connectToWifi(String*, String*);
int setWifiSSID(String);
int setWifiPwd(String);               


//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
SYSTEM_THREAD(ENABLED);
//when using manual mode the user code will run immediately when
//the device is powered on
SYSTEM_MODE(MANUAL);

//STARTUP(WiFi.selectAntenna(ANT_EXTERNAL));

#define WAIT_1_MINUTE 60000;

//blue LED built into photon and argon board = D7
int boardLED = D7;
//external LED connected to D4
int redLED = D4;

//setCredentials() does not store credentials in flash memory unless particle has successfully
//connected to that wifi network in the past, so we need to store credentials using EEPROM

typedef struct Creds {

  //std::string SSID(31,'\0'); ;
  //std::string password(31, '\0');

} Creds ;

String mySSIDs[5] = {"Testbed", "Testbed", "Testbed", "Testbed", "Diagnostics"};
String myPasswords[5] = {"notright1", "fireweed3", "notright3", "notright4", "DiagnosticsPwd"};

// setup() runs once, when the device is first turned on.
void setup() {

  #if defined(DEBUG_BUILD)
  Mesh.off();
  BLE.off();
  #endif

  //set blink LED pins to out 
  pinMode(boardLED,OUTPUT); 
  pinMode(redLED,OUTPUT);

  //start comms with serial terminal for debugging...
  Serial.begin(9600);
  // wait until a character sent from USB host
  Serial.println("Press a key please. ");
  while (!Serial.available()) Particle.process();

  Serial.println("Key press received, starting code...");

/*  std::string myString(31, '\0');
  
  myString = "Testbed";

  int strCap = myString.capacity();
  int strSize = myString.size();

  Serial.println("Capacity     Size");
  Serial.println(strCap,strSize);

  

  int address = 1;
  EEPROM.put(address, mySSIDs);

  String holder;
  EEPROM.get(address,holder);
  Serial.print("holder: ");
  Serial.println(holder);

  int address2 = 2;
  String holder2;
  uint16_t value2;

  EEPROM.get(address2,holder);
  EEPROM.get(address2,value2);
  Serial.print("holder2: ");
  Serial.println(holder2);
  Serial.println(value2);

  if(value2 == 0xFFFF){
    Serial.println("address 8 empty");
  }

  int addr = 1;
  uint16_t value;
  EEPROM.get(addr, value);

  Serial.print("addres 1 value: ");
  Serial.println(value);

  if(value == 0xFFFF){
    Serial.println("address 1 empty");
  }
*/

/*  //loops through 5 different stored networks until connection established
  connectToWifi(mySSIDs,myPasswords);

  //register cloud-connected function before connecting to cloud
  //these will let me change pwd[0] to fireweed4
  Particle.function("changeSSID", setWifiSSID);
  Particle.function("changePwd", setWifiPwd);

  //connect to cloud
  Particle.connect();

  //publish vitals every 5 seconds
  //documentation example says this goes in setup, hokay...
  Particle.publishVitals(5);
*/

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  //has to be called at least once every 20 seconds while in 
  //system manual mode, for... reasons... ???
  //Particle.process();

  Serial.print("you're looping");

  blinkLED();

}

//change wifi SSID from the cloud

//A cloud function is set up to take one argument of the String datatype. 
//This argument length is limited to a max of 622 characters (since 0.8.0). 
//The String is UTF-8 encoded.
int setWifiSSID(String newSSID){

  mySSIDs[0] = newSSID;

  //did it work?
  for(int i = 0; i < 5; i++){
    Serial.print("New credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }


  return -1;

}

int setWifiPwd(String newPwd){

  myPasswords[0] = newPwd;

  //connectToWifi(mySSIDs,myPasswords);

  //did it work?
  for(int i = 0; i < 5; i++){
    Serial.print("New credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);
  }

  return -1;

}

//connects to one of 5 stored wifi networks
void connectToWifi(String* mySSIDs, String* myPasswords){

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
    Serial.print("Setting credential set: ");
    Serial.println(i+1);
    Serial.println(mySSIDs[i]);
    Serial.println(myPasswords[i]);
    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);

    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    
    waitFor(WiFi.ready, 30000);    

    //wifi.ready() returns true when connected and false when not
    if(WiFi.ready()) {
      Serial.println("Connected to wifi.");
      break;
    } else {
      Serial.println("***Failed to connect to wifi***");
      continue;
    }

  }

}  //end connectToWifi()




//uses getCredentials to retrieve and print wifi credentials stored in 
//particle's flash memory
void printWiFiCreds(){

  //hasCredentials returns true if there are wifi credentials stored in flash memory
  if(WiFi.hasCredentials()){

    //WiFiAccessPoint is object with a bunch of typedefs set up to store ssid info etc
    //no docs on objects, have to examine raw source code for details
    WiFiAccessPoint ap[5]; 

    //getCredentials() lists credentials stored in flash, returns # of credentials
    int found = WiFi.getCredentials(ap, 5);

    Serial.print("# of credentials stored: ");
    Serial.println(found);
    Serial.println("");

    for (int i = 0; i < found; i++) {
        Serial.print("credential number: ");
        Serial.println(i+1);
        Serial.print("ssid: ");
        Serial.println(ap[i].ssid);
        // security is one of WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2, WLAN_SEC_WPA_ENTERPRISE, WLAN_SEC_WPA2_ENTERPRISE
        Serial.print("security: ");
        Serial.println(ap[i].security);
        // cipher is one of WLAN_CIPHER_AES, WLAN_CIPHER_TKIP or WLAN_CIPHER_AES_TKIP
        Serial.print("cipher: ");
        Serial.println(ap[i].cipher);
    }

  } else {
    Serial.println("No credentials stored");
  }

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
