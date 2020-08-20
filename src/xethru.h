/*
 * 
 * XeThru libraries:
 * Written by: �yvind Nydal Dahl
 * Company: XeThru / Novelda
 * July 2018
 * 
 * Original code to read data from XeThru and transmit to 
 * Particle cloud written by Sampath Satti, Wayne Ng, and Sajan Rajdev.
 * 
 * XeThru code upgraded to be scaleable by Heidi Fedorak:
 *    -established separate .h and .cpp files for XeThru code
 *    -redefined global variables appropriately
 *    -established odetect_config.h file for all global defines
 *    -removed delay(1000) and adapted PublishData() to control
 *     rate of messages being published to the cloud
 * 
 * 
 */


//*************************defines and global variables that need to be altered during setup*************************

#define USE_SERIAL  //when used, displays serial debugging messages
#define LOCATIONID "HeidiTest"
#define DEVICEID "H"
#define DEVICETYPE "XeThru"

//***************************macro defines******************************

//SERIAL PORTS:
//These definitions work for Arduino Mega, but must be changed for other Arduinos.
//* Note: Using Serial as SerialRadar seems to give a few CRC errors. I'm not seeing this 
//  using Serial1, Serial2, or Serial3. Could probably be solved by changing baud rate)

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



//***************************global variables******************************

//these are used in publishData() only, they can be moved there
char locationid[] = LOCATIONID;
char deviceid[] = DEVICEID;
char devicetype[] = DEVICETYPE;

//this used in setup() only
LEDSystemTheme theme; // Enable custom theme

//used in send_command() and all the xethru_configuration() sub-functions
unsigned char send_buf[TX_BUF_LENGTH];  // Buffer for sending data to radar.

//see logbook notes for where this is used, this one is complicated
//might be easier to leave as global
unsigned char recv_buf[RX_BUF_LENGTH];  // Buffer for receiving data from radar.

//not used anywhere at all, can delete
const char * states[7] = { "Breathing", "Movement", "Movement tracking", "No movement", "Initializing", "", "Unknown" };

 // XeThru configuration variables
 //these can be made defines so they don't have to be global variables
 //they are used (and passed, why, if they're global you shouldn't have to do that, weird...) in xethru_configuration()'s sub-functions
int led = 0;
int noisemap = 0;
int sensitivity = 5;
float min_detect = 0.5;
float max_detect = 4;   

// Struct to hold respiration message from radar
//this legit actually needs to be global
typedef struct RespirationMessage {
  uint32_t state_code;
  float rpm;
  float distance;
  uint32_t signal_quality;  //this is filled from xethru data but never published or used anywhere
  float movement_slow;
  float movement_fast;
  float breathing_pattern;
};

// Initialize arrays and variables
//these are filled with data in loop(), transmitted to cloud and re-set to "" in publishData()
//this makes more sense as a struct which can be initialized in loop()
//could combine with struct above, or could make it a "bulk message" struct?
char distance[500] = "";
char rpm[500] = "";
//breaths -> breathing pattern
char breaths[500] = "";
//slow -> movement slow
char slow[500] = "";
//fast -> movement fast
char fast[500] = "";
//x_state -> state_code
char x_state[500] = "";

//this seems to be used in loop() only, hard to tell though...
int i = 0;

//this doesn't seem to be used anywhere at all, can delete...
int state = 1;

//these three also not used anywhere, can delete...
float prevSOC = 100;
int charging = FALSE;
double rssi;


//***************************function declarations***************


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
    if (!SerialRadar.available())  delay(10);

  } //endwhile

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