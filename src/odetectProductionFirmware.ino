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

// Written by: �yvind Nydal Dahl
// Company: XeThru / Novelda
// July 2018


// SERIAL PORTS:
// These definitions work for Arduino Mega, but must be changed for other Arduinos.
//  * Note: Using Serial as SerialRadar seems to give a few CRC errors. I'm not seeing this 
//    using Serial1, Serial2, or Serial3. Could probably be solved by changing baud rate)
// 



//*************************debugging & code flow defines*************************
#define USE_SERIAL
#define WRITE_ORIGINALS
//#define PHOTON


//*************************global macro defines**********************************

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
//0th SSID/password is used during product setup ONLY
//5th SSID/password is Diagnostics network
#define ADDRSSIDS 0
#define ADDRPWDS 320
#define MAXLEN 64

#define SerialRadar Serial1    // Used for communication with the radar, Serial connection using TX,RX pins
#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB

// Pin definitions
#define RESET_PIN D4 //Datasheet: E8, 26, MICRO_RST_N
#define RADAR_RX_PIN 3  //Datasheet: PA9, 39, MICRO_UART_TX

//
// The following values can be found in XeThru Module Communication Protocol:
//  https://www.xethru.com/community/resources/xethru-module-communication-protocol.130/
//
#define XT_START 0x7d
#define XT_STOP 0x7e
#define XT_ESCAPE 0x7f

#define XTS_ID_SLEEP_STATUS                             (uint32_t)0x2375a16c
#define XTS_ID_RESP_STATUS                              (uint32_t)0x2375fe26
#define XTS_ID_RESPIRATION_MOVINGLIST                   (uint32_t)0x610a3b00
#define XTS_ID_RESPIRATION_DETECTIONLIST                (uint32_t)0x610a3b02
#define XTS_ID_APP_RESPIRATION_2                        (uint32_t)0x064e57ad
#define XTS_ID_DETECTION_ZONE                           (uint32_t)0x96a10a1c
#define XTS_ID_SENSITIVITY                              (uint32_t)0x10a5112b

// Profile codes
#define XTS_VAL_RESP_STATE_BREATHING      0x00 // Valid RPM sensing
#define XTS_VAL_RESP_STATE_MOVEMENT       0x01 // Detects motion, but can not identify breath
#define XTS_VAL_RESP_STATE_MOVEMENT_TRACKING  0x02 // Detects motion, possible breathing soon
#define XTS_VAL_RESP_STATE_NO_MOVEMENT      0x03 // No movement detected
#define XTS_VAL_RESP_STATE_INITIALIZING     0x04 // Initializing sensor
#define XTS_VAL_RESP_STATE_ERROR        0x05 // Sensor has detected some problem. StatusValue indicates problem.
#define XTS_VAL_RESP_STATE_UNKNOWN        0x06 // Undefined state.

#define XTS_SPR_APPDATA 0x50
#define XTS_SPR_SYSTEM 0x30

#define XTS_SPR_DATA 0xA0
#define XTS_SPRD_FLOAT 0x12

#define XTS_SPC_DEBUG_LEVEL 0xb0
#define XTS_SPC_APPCOMMAND 0x10
#define XTS_SPC_MOD_SETMODE 0x20
#define XTS_SPC_MOD_LOADAPP 0x21
#define XTS_SPC_MOD_RESET 0x22
#define XTS_SPC_MOD_SETCOM 0x23
#define XTS_SPC_MOD_SETLEDCONTROL 0x24
#define XTS_SPC_MOD_NOISEMAP 0x25

// Output control
#define XTID_OUTPUT_CONTROL_DISABLE    (0)
#define XTID_OUTPUT_CONTROL_ENABLE     (1)

// Sensor mode IDs
#define XTS_SM_RUN                  (0x01)
#define XTS_SM_NORMAL               (0x10)
#define XTS_SM_IDLE                 (0x11)
#define XTS_SM_MANUAL               (0x12)
#define XTS_SM_STOP                 (0x13)

#define XTS_SPR_ACK 0x10
#define XTS_SPR_ERROR 0x20

#define XTS_SPRS_BOOTING (uint32_t)0x00000010
#define XTS_SPRS_READY (uint32_t)0x00000011

#define XTS_SPCA_SET 0x10
#define XTS_SPCN_SETCONTROL 0x10
#define XTS_SPCO_SETCONTROL 0x10
#define XTS_SPC_OUTPUT 0x41

#define TX_BUF_LENGTH 64
#define RX_BUF_LENGTH 64

#define DATA_PER_MESSAGE 1

#define STATE_NO_PRESENCE   0
#define STATE_MOVEMENT      1
#define STATE_BREATH_TRACK  2
#define STATE_OD            3

#define RPM_THRESHOLD 15



//*************************global variable defines*******************

char locationid[] = "HeidiTest";
char deviceid[] = "H";
char devicetype[] = "XeThru";

//set initial SSID/password pairs here
char mySSIDs[5][64] = {"wrongssid123456789", "Testbed", "Testbed3", "Testbed4", "Diagnostics"};
char myPasswords[5][64] = {"notright123456789", "fireweed4", "notright3", "notright4", "DiagnosticsPwd"};

LEDSystemTheme theme; // Enable custom theme
unsigned char send_buf[TX_BUF_LENGTH];  // Buffer for sending data to radar. 
unsigned char recv_buf[RX_BUF_LENGTH];  // Buffer for receiving data from radar.
const char * states[7] = { "Breathing", "Movement", "Movement tracking", "No movement", "Initializing", "", "Unknown" };

 // XeThru configuration variables
int led = 0;
int noisemap = 0;
int sensitivity = 5;
float min_detect = 0.5;
float max_detect = 4;   

// Struct to hold respiration message from radar
typedef struct RespirationMessage {
  uint32_t state_code;
  float rpm;
  float distance;
  uint32_t signal_quality;
  float movement_slow;
  float movement_fast;
  float breathing_pattern;
};

//int get_respiration_data(RespirationMessage*); //- extra f/n declaration I've duplicated in the correct section above

// Initialize arrays and variables
char distance[500] = "";
char rpm[500] = "";
char breaths[500] = "";
char slow[500] = "";
char fast[500] = "";
char x_state[500] = "";
int i = 0;
int state = 1;
float prevSOC = 100;
int charging = FALSE;

double rssi;



//*************************function declarations*************************

void blinkLED();
void connectToWifi(char array1[][64], char array2[][64]);
int setWifiSSID(String);
int setWifiPwd(String);   
void writeToFlash();        
void readFromFlash(); 
//char* truncate(char mySSIDs[][MAXLEN]);

void xethru_reset();
void xethru_configuration();
int get_configuration_values(String command);
void publishData();
int get_respiration_data(RespirationMessage * resp_msg);
void stop_module();
void set_sensitivity(uint32_t sensitivity);
void set_detection_zone(float zone_start, float zone_end);
void run_profile();
void set_debug_level();
void set_led_control(uint32_t control);
void load_profile(uint32_t profile);
void configure_noisemap(uint32_t code);
void enable_output_message(uint32_t message);
void disable_output_message(uint32_t message);
void errorPublish(String message);
void wait_for_ready_message();
void get_ack();
void send_command(int len);
int receive_data();


//*************************System/Startup messages for Particle API***********

//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
SYSTEM_THREAD(ENABLED);
//when using manual mode the user code will run immediately when
//the device is powered on
SYSTEM_MODE(MANUAL);

#if defined(PHOTON)
STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna
#endif


// setup() runs once, when the device is first turned on.
void setup() {

  #if defined(DEBUG_BUILD)
  Mesh.off();
  BLE.off();
  #endif

  pinMode(D6, INPUT_PULLUP);  //XeThru code


  #if defined(USE_SERIAL)
  //start comms with serial terminal for debugging...
  SerialDebug.begin(115200);
  // wait until a character sent from USB host
  waitUntil(SerialDebug.available);

  SerialDebug.println("Key press received, starting code...");
  #endif

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
  Particle.function("config", get_configuration_values); //XeThru code

  //connect to cloud
  Particle.connect();            

  //publish vitals every 5 seconds
  //documentation example says this goes in setup, hokay...
  //Particle.publishVitals(60); -> from XeThru code
  Particle.publishVitals(60);


  //***XeThru code from here to end of setup()
  theme.setColor(LED_SIGNAL_CLOUD_CONNECTED, 0x00000000); // Set LED_SIGNAL_NETWORK_ON to no color
  theme.apply(); // Apply theme settings 
	
  xethru_reset();
  
  /*
  // Initializes the XeThru with previously set configuration values if they exist
  if(EEPROM.get(1) == 5) {
      led = EEPROM.get(2);
      noisemap = EEPROM.get(6);
      sensitivity = EEPROM.get(10);
      min_detect = EEPROM.get(14);
      max_detect = EEPROM.get(18);
  }
  */
  
  xethru_configuration();

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  #if defined(USE_SERIAL)
  SerialDebug.print("you're looping");
  #endif

  //WiFi.ready = false if wifi is lost. If false, try to reconnect
  if(!WiFi.ready()){
    connectToWifi(mySSIDs, myPasswords);
  }  

  // For every loop we check to see if we have received any respiration data

    RespirationMessage msg;
    if(get_respiration_data(&msg)) {
          
        if(msg.state_code != XTS_VAL_RESP_STATE_INITIALIZING || msg.state_code != XTS_VAL_RESP_STATE_ERROR || msg.state_code != XTS_VAL_RESP_STATE_UNKNOWN) {
            // Appends the data received to the end of the array
            if(i%DATA_PER_MESSAGE!=0) {
                strcat(rpm, String(msg.rpm)+" ");
                strcat(distance, String(msg.distance)+" ");
                strcat(breaths, String(msg.breathing_pattern)+" ");
                strcat(slow, String(msg.movement_slow)+" ");
                strcat(fast, String(msg.movement_fast)+" ");
                strcat(x_state, String(msg.state_code)+" ");     
                i++;
            }
            // If the number of data points is enough, append the data without a space and publish the data to the Particle Cloud.
            if(i%DATA_PER_MESSAGE==0) {
                
                strcat(rpm, String(msg.rpm));
                strcat(distance, String(msg.distance));
                strcat(breaths, String(msg.breathing_pattern));
                strcat(slow, String(msg.movement_slow));
                strcat(fast, String(msg.movement_fast));
                strcat(x_state, String(msg.state_code));
                publishData();
                i++;
                //delay(1000);
            }
        }
    }
  delay(1000);

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
    SerialDebug.print("Setting credential set: ");
    SerialDebug.println(i+1);
    SerialDebug.println(mySSIDs[i]);
    SerialDebug.println(myPasswords[i]);
    #endif

    //WiFi.setCredentials(ssidHolder,pwdHolder);

    WiFi.setCredentials(mySSIDs[i],myPasswords[i]);

    WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);  
    
    waitFor(WiFi.ready, 30000);    

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

  strcpy(mySSIDs[0],newSSID.c_str());

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


  return -1;

}

int setWifiPwd(String newPwd){

  const char* pwdHolder = newPwd.c_str();

  strcpy(myPasswords[0], pwdHolder);
 
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

  connectToWifi(mySSIDs,myPasswords);

  //it's never going to return because connectToWifi() above kills connection
  return -1;

}

//***************************XeThru functions**********************************



void xethru_reset() {
  //
  // If the RX-pin of the radar is low during reset or power-up, it goes into bootloader mode.
  // We don't want that, that's why we first set the RADAR_RX_PIN to high, then reset the module.
  pinMode(RADAR_RX_PIN, OUTPUT);
  digitalWrite(RADAR_RX_PIN, HIGH);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
}



void xethru_configuration() {

  // Set up serial communication
  SerialRadar.begin(115200);

  // After the module resets, the XTS_SPRS_BOOTING message is sent. Then, after the 
  // module booting sequence is completed and the module is ready to accept further
  // commands, the XTS_SPRS_READY command is issued. Let's wait for this.
  wait_for_ready_message();

  // Stop the module, in case it is running
  stop_module();

  // Set debug level
  set_debug_level();
  // Load respiration profile  
  load_profile(XTS_ID_APP_RESPIRATION_2);

   // Configure the noisemap
  configure_noisemap(noisemap);
  
  // Set LED control
  set_led_control(led); // 0: OFF; 1: SIMPLE; 2: FULL

  // Set detection zone
  set_detection_zone(min_detect, max_detect); // First variable = Lower limit, Second variable = Upper limit

  // Set sensitivity
  set_sensitivity(sensitivity);


  // Enable only the Sleep message, disable all others
  enable_output_message(XTS_ID_SLEEP_STATUS);
  disable_output_message(XTS_ID_RESP_STATUS);
  disable_output_message(XTS_ID_RESPIRATION_MOVINGLIST);
  disable_output_message(XTS_ID_RESPIRATION_DETECTIONLIST);

  // Run profile - after this the radar will start sending the sleep message we enabled above
  run_profile();
}




int get_configuration_values(String command) { // command is a long string with all the config values
    // Parse the command
    int split1 = command.indexOf(',');
    led = command.substring(0,split1).toInt();
    int split2 = command.indexOf(',', split1+1);
    noisemap = command.substring(split1+1,split2).toInt();
    int split3 = command.indexOf(',', split2+1);
    sensitivity = command.substring(split2+1,split3).toInt();
    int split4 = command.indexOf(',', split3+1);
    min_detect = command.substring(split3+1,split4).toFloat();
    int split5 = command.indexOf(',', split4+1);
    max_detect = command.substring(split4+1,split5).toFloat();

    Particle.publish("min_detect", String(min_detect));
    Particle.publish("max_detect", String(max_detect));

    xethru_reset();
    xethru_configuration();
    
    /*
    //Saves the values into the EEPROM so it is initialized with the most recent values if the Photon gets reset
    //sets a value for the first EEPROM byte so it does not use default values if reset
    EEPROM.put(1, 5);
    //each of the variables is 4 bytes long
    EEPROM.put(2, led);
    EEPROM.put(6, noisemap);
    EEPROM.put(10, sensitivity);
    EEPROM.put(14, min_detect);
    EEPROM.put(18, max_detect);
    */
    
    return 1;
}








// Takes the data received from the XeThru message and publishes it to the Particle Cloud
// There is a webhook set up to send the data to Firebase Database from the event trigger of the publish
void publishData() {

    char buf[1024];
    // The data values can't be inserted on the publish message so it must be printed into a buffer first.
    // The backslash is used as an escape character for the quotation marks.
    
//	snprintf(buf, sizeof(buf), "{\"a\":%d, \"b\":%s}", data, *data);
	snprintf(buf, sizeof(buf), "{\"devicetype\":\"%s\", \"location\":\"%s\", \"device\":\"%s\", \"distance\":\"%s\", \"rpm\":\"%s\", \"slow\":\"%s\", \"fast\":\"%s\", \"state\":\"%s\"}", devicetype, locationid, deviceid, distance, rpm, slow, fast, x_state);
//	snprintf(buf, sizeof(buf), "{\"distance\":\"%s\", \"rpm\":\"%s\", \"breaths\":\"%s\"}", distance, rpm, breaths);
//	Serial.printlnf("publishing %s", buf);
//	Particle.publish("XeThru", buf, PRIVATE);
    Particle.publish("Sleep", buf, PRIVATE);
	
    //Particle.publish("data", String(data));
    //strcpy(data1, "");
    
    //Clears the data arrays to be filled again
    strcpy(rpm, "");
    strcpy(distance, "");
    strcpy(breaths, "");
    strcpy(slow, "");
    strcpy(fast, "");
    strcpy(x_state, "");
}


int get_respiration_data(RespirationMessage * resp_msg) {

  // receive_data() fills recv_buf[] with data.
  if (receive_data() < 1) {
      // Particle.publish("check", "This is not receiving data");
      return 0;
  }
  
  // Respiration Sleep message format:
  //
  // <Start> + <XTS_SPR_APPDATA> + [XTS_ID_SLEEP_STATUS(i)] + [Counter(i)]
  // + [StateCode(i)] + [RespirationsPerMinute(f)] + [Distance(f)]
  // + [SignalQuality(i)] + [MovementSlow(f)] + [MovementFast(f)]
  //
  
  // Check that it's a sleep message (XTS_ID_SLEEP_STATUS)
  uint32_t xts_id = *((uint32_t*)&recv_buf[2]);
  if (xts_id == XTS_ID_SLEEP_STATUS) {
  
      // Extract the respiration message data:
      resp_msg->state_code = *((uint32_t*)&recv_buf[10]);
      resp_msg->rpm = *((float*)&recv_buf[14]);
      resp_msg->distance = *((float*)&recv_buf[18]);
      resp_msg->signal_quality = *((uint32_t*)&recv_buf[22]);
      resp_msg->movement_slow = *((float*)&recv_buf[26]);
      resp_msg->movement_fast = *((float*)&recv_buf[30]);
      
      //Particle.publish("id1", String(xts_id));
      // Return OK
      return 1;
  }
  if (xts_id == XTS_ID_RESP_STATUS) {
      
      // Extract the respiration message data:
      resp_msg->state_code = *((uint32_t*)&recv_buf[10]);
      resp_msg->rpm = *((uint32_t*)&recv_buf[14]);                 //State_data (RPM) according to datasheet
      resp_msg->distance = *((float*)&recv_buf[18]);
      resp_msg->breathing_pattern = *((float*)&recv_buf[22]);
      resp_msg->signal_quality = *((uint32_t*)&recv_buf[26]);
      
      //Particle.publish("id2", String(xts_id));
      // Return OK
      return 1;
  }
  //return 1;
}


// Stop module from running
void stop_module() 
{
  // Empty the buffer before stopping the radar profile:
  while (SerialRadar.available())
    SerialRadar.read();
    
  // Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_MOD_SETMODE;
  send_buf[2] = XTS_SM_STOP;
  
  // Send the command
  send_command(3);

  // Get ACK response from radar
  get_ack();
}


// Set sensitivity
void set_sensitivity(uint32_t sensitivity) 
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_APPCOMMAND;
  send_buf[2] = XTS_SPCA_SET;
  send_buf[3] = XTS_ID_SENSITIVITY & 0xff;
  send_buf[4] = (XTS_ID_SENSITIVITY >> 8) & 0xff;
  send_buf[5] = (XTS_ID_SENSITIVITY >> 16) & 0xff;  
  send_buf[6] = (XTS_ID_SENSITIVITY >> 24) & 0xff;
  send_buf[7] = sensitivity & 0xff;
  send_buf[8] = (sensitivity >> 8) & 0xff;
  send_buf[9] = (sensitivity >> 16) & 0xff;  
  send_buf[10] = (sensitivity >> 24) & 0xff;
  
  //Send the command
  send_command(11);
  
  // Get ACK response from radar
  get_ack();
}



// Set detection zone
void set_detection_zone(float zone_start, float zone_end) 
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_APPCOMMAND;
  send_buf[2] = XTS_SPCA_SET;
  send_buf[3] = XTS_ID_DETECTION_ZONE & 0xff;
  send_buf[4] = (XTS_ID_DETECTION_ZONE >> 8) & 0xff;
  send_buf[5] = (XTS_ID_DETECTION_ZONE >> 16) & 0xff;
  send_buf[6] = (XTS_ID_DETECTION_ZONE >> 24) & 0xff;
  
  // Copy the bytes of the floats to send buffer
  memcpy(send_buf+7, &zone_start, 4);
  memcpy(send_buf+11, &zone_end, 4);
  
  //Send the command
  send_command(15);
  
  // Get ACK response from radar
  get_ack();
}


  
// Run profile
void run_profile() 
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_MOD_SETMODE;
  send_buf[2] = XTS_SM_RUN;

  //Send the command
  send_command(3);
  
  // Get ACK response from radar
  get_ack();
}

// Set Debug Level
void set_debug_level() 
{
  // Bit 0: None
  // Bit 1: Error
  // Bit 2: Warning
  // Bit 3: Info
  // Bit 4: Debug
    
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_DEBUG_LEVEL;
  send_buf[2] = 0x1f;

  //Send the command
  send_command(3);
  
  // Get ACK response from radar
  get_ack();
}

// Set Debug Level
void set_led_control(uint32_t control) 
{
  // For byte index 2
  // 0: OFF
  // 1: SIMPLE
  // 2: FULL
    
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_MOD_SETLEDCONTROL;
  send_buf[2] = control & 0xff;

  //Send the command
  send_command(3);
  
  // Get ACK response from radar
  get_ack();
}

// Load profile
void load_profile(uint32_t profile)
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_MOD_LOADAPP;
  send_buf[2] = profile & 0xff;
  send_buf[3] = (profile >> 8) & 0xff;
  send_buf[4] = (profile >> 16) & 0xff;  
  send_buf[5] = (profile >> 24) & 0xff;
  
  //Send the command
  send_command(6);
  
  // Get ACK response from radar
  get_ack();
}

void configure_noisemap(uint32_t code) 
{
  // send_buf[3] Configuration:
  //
  // Bit 0: FORCE INITIALIZE NOISEMAP ON RESET
  // Bit 1: ADAPTIVE NOISEMAP ON
  // Bit 2: USE DEFAULT NOISEMAP
  // 

  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_MOD_NOISEMAP;
  send_buf[2] = XTS_SPCN_SETCONTROL;
  send_buf[3] = code & 0xff; // 0x06: Use default noisemap and adaptive noisemap
  send_buf[4] = 0x00;
  send_buf[5] = 0x00;
  send_buf[6] = 0x00;

  //Send the command
  send_command(7);
  
  // Get ACK response from radar
  get_ack();
}


void enable_output_message(uint32_t message) 
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_OUTPUT;
  send_buf[2] = XTS_SPCO_SETCONTROL;
  send_buf[3] = message & 0xff;
  send_buf[4] = (message >> 8) & 0xff;
  send_buf[5] = (message >> 16) & 0xff;
  send_buf[6] = (message >> 24) & 0xff;
  send_buf[7] = XTID_OUTPUT_CONTROL_ENABLE & 0xff;
  send_buf[8] = (XTID_OUTPUT_CONTROL_ENABLE >> 8) & 0xff;
  send_buf[9] = (XTID_OUTPUT_CONTROL_ENABLE >> 16) & 0xff;
  send_buf[10] = (XTID_OUTPUT_CONTROL_ENABLE >> 24) & 0xff;

  //Send the command
  send_command(11);
  
  // Get ACK response from radar
  get_ack();
}


void disable_output_message(uint32_t message) 
{
  //Fill send buffer
  send_buf[0] = XT_START;
  send_buf[1] = XTS_SPC_OUTPUT;
  send_buf[2] = XTS_SPCO_SETCONTROL;
  send_buf[3] = message & 0xff;
  send_buf[4] = (message >> 8) & 0xff;
  send_buf[5] = (message >> 16) & 0xff;
  send_buf[6] = (message >> 24) & 0xff;
  send_buf[7] = XTID_OUTPUT_CONTROL_DISABLE & 0xff;
  send_buf[8] = (XTID_OUTPUT_CONTROL_DISABLE >> 8) & 0xff;
  send_buf[9] = (XTID_OUTPUT_CONTROL_DISABLE >> 16) & 0xff;
  send_buf[10] = (XTID_OUTPUT_CONTROL_DISABLE >> 24) & 0xff;

  //Send the command
  send_command(11);
  
  // Get ACK response from radar
  get_ack();
}

// Publishes an error message to be stored in database
void errorPublish(String message) {
    Particle.publish("Error", message, PRIVATE);
}



// This method waits indefinitely for the XTS_SPRS_READY message from the radar
void wait_for_ready_message()
{
  SerialDebug.println("Waiting for XTS_SPRS_READY...");
  while (true) {
    if (receive_data() < 1)
      continue;

    if (recv_buf[1] != XTS_SPR_SYSTEM)
      continue;

    //uint32_t response_code = (uint32_t)recv_buf[2] | ((uint32_t)recv_buf[3] << 8) | ((uint32_t)recv_buf[4] << 16) | ((uint32_t)recv_buf[5] << 24);
    uint32_t response_code = *((uint32_t*)&recv_buf[2]);
    if (response_code == XTS_SPRS_READY) {
      SerialDebug.println("Received XTS_SPRS_READY!");
      return;
    }
    else if (response_code == XTS_SPRS_BOOTING)
      SerialDebug.println("Radar is booting...");
  }
}

// This method checks if an ACK was received from the radar
void get_ack()
{
  int len = receive_data();
  
  if (len == 0) {
    SerialDebug.println("No response from radar");
    errorPublish("No reponse from radar");
  }
  else if (len < 0) {
    SerialDebug.println("Error in response from radar");
    errorPublish("Error in response from radar");
  }
  else if (recv_buf[1] != XTS_SPR_ACK) {  // Check response for ACK
    SerialDebug.println("Did not receive ACK!");
    errorPublish("ACK not received");
  }
}


/*
 * Adds CRC, Escaping and Stop byte to the
 * send_buf and sends it over the SerialRadar.
 */
void send_command(int len) 
{ 
  unsigned char crc = 0;
  
  // Calculate CRC
  for (int i = 0; i < len; i++)
    crc ^= send_buf[i];

  // Add CRC to send buffer
  send_buf[len] = crc;
  len++;
  
  // Go through send buffer and add escape characters where needed
  for (int i = 1; i < len; i++) {
    if (send_buf[i] == XT_START || send_buf[i] == XT_STOP || send_buf[i] == XT_ESCAPE)
    {
      // Shift following bytes one place up
      for (int u=len; u > i; u--)
        send_buf[u] = send_buf[u-1];

      // Add escape byte at old byte location
      send_buf[i] = XT_ESCAPE;

      // Increase length by one
      len++;
      
      // Increase counter so we don't process it again
      i++;
    }
  } 
  
  // Send data (including CRC) and XT_STOP
  SerialRadar.write(send_buf, len);
  SerialRadar.write(XT_STOP);

  // Print out sent data for debugging:
  SerialDebug.print("Sent: ");  
  for (int i = 0; i < len; i++) {
    SerialDebug.print(send_buf[i], HEX);  
    SerialDebug.print(" ");
  }
  SerialDebug.println(XT_STOP, HEX);
}
  
  
/* 
 * Receive data from radar module
 *  -Data is stored in the global array recv_buf[]
 *  -On success it returns number of bytes received (without escape bytes
 *  -On error it returns -1
 */
int receive_data() {

  int recv_len = 0;  //Number of bytes received

  // Wait 500 ms if nothing is available yet
  if (!SerialRadar.available())
    delay(500);
    
  // Wait for start character
  while (SerialRadar.available()) 
  {
    unsigned char c = SerialRadar.read();  // Get one byte from radar

    // If we receive an ESCAPE character, the next byte is never the real start character
    if (c == XT_ESCAPE)
    {
      // Wait for next byte and skip it.
      while (!SerialRadar.available());
      SerialRadar.read();
    }
    else if (c == XT_START) 
    {
      // If it's the start character we fill the first character of the buffer and move on
      recv_buf[0] = c;
      recv_len = 1;
      break;
    }

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())
      delay(10);
  }

  // Wait 10 ms if nothing is available yet
  if (!SerialRadar.available())
    delay(10);
    
  // Start receiving the rest of the bytes
  while (SerialRadar.available()) 
  {
    // read a byte
    unsigned char c = SerialRadar.read(); // Get one byte from radar

    // is it an escape byte?
    if (c == XT_ESCAPE)
    {
      // If it's an escape character next character in buffer is data and not special character:
      while (!SerialRadar.available());
      c = SerialRadar.read();
    }
    // is it the stop byte?
    else if (c == XT_STOP) {
      // Fill response buffer, and increase counter
      recv_buf[recv_len++] = c;
      break;  //Exit this loop 
    }

    if (recv_len >= RX_BUF_LENGTH) {
      errorPublish("Buffer Overflow");
      SerialDebug.println("BUFFER OVERFLOW!");
      return -1;
    }
    
    // Fill response buffer, and increase counter
    recv_buf[recv_len++] = c;

    

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())
      delay(10);
  }

  // Print received data
  #if 0
  SerialDebug.print("Received: ");
  for (int i = 0; i < recv_len; i++) {
    SerialDebug.print(recv_buf[i], HEX);
    SerialDebug.print(" ");
  }
  SerialDebug.println(" ");
  #endif
  
  // If nothing was received, return 0.
  if (recv_len==0) {
    errorPublish("No message received");
    return 0; //Many sleep messages at once cause no message sometimes.
  }
  // If stop character was not received, return with error.
  if (recv_buf[recv_len-1] != XT_STOP) {
    recv_buf[recv_len++] = XT_STOP; // If the message is missing the Stop Byte, just append it at the end.
    //char boof[1024];
	//snprintf(boof, sizeof(boof), "Length of received buffer: %d", recv_len);
    //Particle.publish("Length", boof, PRIVATE); //Checks how far into the message it gets
    //return -1; //Too many legacy messages has some messages missing stop byte.
  }


  //
  // Calculate CRC
  //
  unsigned char crc = 0;
  
  // CRC is calculated without the crc itself and the stop byte, hence the -2 in the counter
  for (int i = 0; i < recv_len-2; i++) 
    crc ^= recv_buf[i];
  
  // Check if calculated CRC matches the recieved
  if (crc == recv_buf[recv_len-2]) 
  {
    return recv_len;  // Return length of received data (without escape bytes) upon success
  }
  else 
  {
    errorPublish("CRC mismatch");
    SerialDebug.print("CRC mismatch: ");
    SerialDebug.print(crc, HEX);
    SerialDebug.print(" != ");
    SerialDebug.println(recv_buf[recv_len-2], HEX);
    return -1; // Return -1 upon crc failure
  } 
}