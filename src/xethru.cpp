#include "Particle.h"
#include "odetect_config.h"
#include "xethru.h"

//******************global variable initialization*******************

//these don't need to be global, I think, but they are used so often it would 
//take more time than I'm willing to spend fixing it. May revisit in the future.
unsigned char xethru_send_buf[TX_BUF_LENGTH];  // Buffer for sending data to radar.
unsigned char xethru_recv_buf[RX_BUF_LENGTH];  // Buffer for receiving data from radar.

//***************************XeThru functions**********************************

/*****************called from loop() or loop() sub-functions*******************/

//called from loop(), this is the main operation of the Xethru device
void checkXethru(){  

  static RespirationMessage msg;

  //if we get a respiration message
  if(get_respiration_data(&msg)) {
    
    //and if the message is of the right type
    if(msg.state_code != XTS_VAL_RESP_STATE_INITIALIZING || msg.state_code != XTS_VAL_RESP_STATE_ERROR || msg.state_code != XTS_VAL_RESP_STATE_UNKNOWN) {

      publishXethruData(&msg);
        
    }//end if message is correct

  }//end if there is a message

}

//called from checkXethru() which is in turn called from loop()
//reads a single respiration message from the xethru
//returns 1 if successful, 0 if not successful
int get_respiration_data(RespirationMessage* resp_msg) {

  // receive_data() fills xethru_recv_buf[] with data.
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
  uint32_t xts_id = *((uint32_t*)&xethru_recv_buf[2]);
  if (xts_id == XTS_ID_SLEEP_STATUS) {
  
      // Extract the respiration message data:
      resp_msg->state_code = *((uint32_t*)&xethru_recv_buf[10]);
      resp_msg->rpm = *((float*)&xethru_recv_buf[14]);
      resp_msg->distance = *((float*)&xethru_recv_buf[18]);
      resp_msg->signal_quality = *((uint32_t*)&xethru_recv_buf[22]);
      resp_msg->movement_slow = *((float*)&xethru_recv_buf[26]);
      resp_msg->movement_fast = *((float*)&xethru_recv_buf[30]);
      
      //Particle.publish("id1", String(xts_id));
      // Return OK
      return 1;
  }
  if (xts_id == XTS_ID_RESP_STATUS) {
      
      // Extract the respiration message data:
      resp_msg->state_code = *((uint32_t*)&xethru_recv_buf[10]);
      resp_msg->rpm = *((uint32_t*)&xethru_recv_buf[14]);                 //State_data (RPM) according to datasheet
      resp_msg->distance = *((float*)&xethru_recv_buf[18]);
      resp_msg->breathing_pattern = *((float*)&xethru_recv_buf[22]);
      resp_msg->signal_quality = *((uint32_t*)&xethru_recv_buf[26]);
      
      //Particle.publish("id2", String(xts_id));
      // Return OK
      return 1;
  }
  return 0;
}

//called from checkXethru(), which is in turn called from loop()
// Takes the data received from the XeThru message and publishes it to the Particle Cloud
// There is a webhook set up to send the data to Firebase Database from the event trigger of the publish
void publishXethruData(RespirationMessage* message) {

  char locationid[] = LOCATIONID;
  char deviceid[] = DEVICEID;
  char devicetype[] = DEVICETYPE;

  char buf[1024];
  // The data values can't be inserted on the publish message so it must be printed into a buffer first.
  // The backslash is used as an escape character for the quotation marks.
  snprintf(buf, sizeof(buf), "{\"devicetype\":\"%s\", \"location\":\"%s\", \"device\":\"%s\", \"distance\":\"%f\", \"rpm\":\"%f\", \"slow\":\"%f\", \"fast\":\"%f\", \"state\":\"%lu\"}", 
        devicetype, locationid, deviceid, message->distance, message->rpm, message->movement_slow, message->movement_fast, message->state_code);

  Particle.publish("XeThru", buf, PRIVATE);  
}


/*****************called from setup() or setup() sub-functions*******************/

//called from setup(), contains xethru-specific setup
void xethruSetup(){

  pinMode(D6, INPUT_PULLUP); 
  //LEDSystemTheme theme; // Enable custom theme
  //theme.setColor(LED_SIGNAL_CLOUD_CONNECTED, 0x00000000); // Set LED_SIGNAL_NETWORK_ON to no color
  //theme.apply(); // Apply theme settings 
	
  XeThruConfigSettings xethruConfig;

  xethruConfig = init_XeThruConfigSettings();

  xethru_reset();
  xethru_configuration(&xethruConfig);

}

//called from xethruSetup()
XeThruConfigSettings init_XeThruConfigSettings(){

   //read the first two bytes of memory. Particle docs say all
  //bytes of flash initialized to OxF. First two bytes are 0xFFFF
  //on new boards, note 0xFFFF does not correspond to any ASCII chars
  #if defined(WRITE_ORIGINAL_XETHRU)
  EEPROM.put(ADDR_XETHRUCONFIG,0xFFFF);
  #endif

  XeThruConfigSettings xethruConfig;

  uint16_t checkForContent;
  EEPROM.get(ADDR_XETHRUCONFIG,checkForContent);

  //if memory has not been written to yet, write the original settings
  //If memory has been written to then console function to update 
  //settings has been called before, so read what was written there
  if(checkForContent == 0xFFFF) {
    XeThruConfigSettings holder;
    initOriginals(&holder);
    writeXethruToFlash(&holder);
  } else {
    xethruConfig = readXethruFromFlash();
  }

  #if defined(WRITE_ORIGINAL_XETHRU)
  xethruConfig = readXethruFromFlash();
  #endif

  #if defined(SERIAL_DEBUG)
    SerialDebug.println("xethruConfig at end of init_xethru");
    SerialDebug.printlnf("led: %d, max: %f, min: %f, noisemap: %d, sensitivity: %d",
            xethruConfig.led,xethruConfig.max_detect,xethruConfig.min_detect,xethruConfig.noisemap,xethruConfig.sensitivity);
  #endif

  return xethruConfig;

}

//called from xethruSetup() 
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

//called from xethruSetup()
void xethru_configuration(XeThruConfigSettings* configSettings) {

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
  configure_noisemap(configSettings->noisemap);
  
  // Set LED control
  set_led_control(configSettings->led); // 0: OFF; 1: SIMPLE; 2: FULL

  // Set detection zone
  set_detection_zone(configSettings->min_detect, configSettings->max_detect); // First variable = Lower limit, Second variable = Upper limit

  // Set sensitivity
  set_sensitivity(configSettings->sensitivity);

  // Enable only the Sleep message, disable all others
  enable_output_message(XTS_ID_SLEEP_STATUS);
  disable_output_message(XTS_ID_RESP_STATUS);
  disable_output_message(XTS_ID_RESPIRATION_MOVINGLIST);
  disable_output_message(XTS_ID_RESPIRATION_DETECTIONLIST);

  // Run profile - after this the radar will start sending the sleep message we enabled above
  run_profile();
}

/*****************particle console functions********************************/

//particle console function to get new xethru config values
int xethruConfigValesFromConsole(String command) { // command is a long string with all the config values

  const char* checkForEcho = command.c_str();
  if(*checkForEcho == 'e'){
    XeThruConfigSettings holder = readXethruFromFlash();
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "{\"led\":\"%d\", \"max_detect\":\"%f\", \"min_detect\":\"%f\", \"noisemap\":\"%d\", \"sensitivity\":\"%d\"}", 
            holder.led,holder.max_detect,holder.min_detect,holder.noisemap,holder.sensitivity); 
    Particle.publish("Current Xethru config settings",buffer,PRIVATE);
  } else //else we have a command to parse
  {
    // command is in the form "led,noisemap,sensitivity,min_detect,max_detect"
    // where variable names are replaced with appropriate numbers
    // 1, 2, 3, 2.5, 3.5
    XeThruConfigSettings newConfig;
    int split1 = command.indexOf(',');
    newConfig.led = command.substring(0,split1).toInt();
    int split2 = command.indexOf(',', split1+1);
    newConfig.noisemap = command.substring(split1+1,split2).toInt();
    int split3 = command.indexOf(',', split2+1);
    newConfig.sensitivity = command.substring(split2+1,split3).toInt();
    int split4 = command.indexOf(',', split3+1);
    newConfig.min_detect = command.substring(split3+1,split4).toFloat();
    int split5 = command.indexOf(',', split4+1);
    newConfig.max_detect = command.substring(split4+1,split5).toFloat();

    
    writeXethruToFlash(&newConfig);

    #if defined(SERIAL_DEBUG)
    //did it get written correctly?
    XeThruConfigSettings holder = readXethruFromFlash();
      SerialDebug.println("xethruConfig after console function called:");
      SerialDebug.printlnf("led: %d, max: %f, min: %f, noisemap: %d, sensitivity: %d",
              holder.led,holder.max_detect,holder.min_detect,holder.noisemap,holder.sensitivity); 
    #endif 

    xethru_reset();
    xethru_configuration(&newConfig);
  } //end if-else

  return 1;

}

/**********sub-functions for all of the above***************/

void initOriginals(XeThruConfigSettings* xethruConfigPtr){

  xethruConfigPtr->led = XETHRU_LED_SETTING;
  xethruConfigPtr->noisemap = XETHRU_NOISEMAP_SETTING;
  xethruConfigPtr->sensitivity = XETHRU_SENSITIVITY_SETTING;
  xethruConfigPtr->min_detect = XETHRU_MIN_DETECT_SETTING;
  xethruConfigPtr->max_detect = XETHRU_MAX_DETECT_SETTING;

}

void writeXethruToFlash(XeThruConfigSettings* xethruConfigPtr) {

  //EEPROM.put() will compare object data to data currently in EEPROM
  //to avoid re-writing values that haven't changed
  //passing put() dereferenced pointer to config struct
  EEPROM.put(ADDR_XETHRUCONFIG,*xethruConfigPtr);  

}

XeThruConfigSettings readXethruFromFlash() {

  XeThruConfigSettings holder;

  EEPROM.get(ADDR_XETHRUCONFIG,holder);

   #if defined(SERIAL_DEBUG)
   //did it get written correctly?
    SerialDebug.println("xethruConfig in read function:");
    SerialDebug.printlnf("led: %d, max: %f, min: %f, noisemap: %d, sensitivity: %d",
            holder.led,holder.max_detect,holder.min_detect,holder.noisemap,holder.sensitivity); 
  #endif 

  return holder;  

}



// Stop module from running
void stop_module() 
{
  // Empty the buffer before stopping the radar profile:
  while (SerialRadar.available())
    SerialRadar.read();
    
  // Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_MOD_SETMODE;
  xethru_send_buf[2] = XTS_SM_STOP;
  
  // Send the command
  send_command(3);

  // Get ACK response from radar
  get_ack();
}


// Set sensitivity
void set_sensitivity(uint32_t sensitivity) 
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_APPCOMMAND;
  xethru_send_buf[2] = XTS_SPCA_SET;
  xethru_send_buf[3] = XTS_ID_SENSITIVITY & 0xff;
  xethru_send_buf[4] = (XTS_ID_SENSITIVITY >> 8) & 0xff;
  xethru_send_buf[5] = (XTS_ID_SENSITIVITY >> 16) & 0xff;  
  xethru_send_buf[6] = (XTS_ID_SENSITIVITY >> 24) & 0xff;
  xethru_send_buf[7] = sensitivity & 0xff;
  xethru_send_buf[8] = (sensitivity >> 8) & 0xff;
  xethru_send_buf[9] = (sensitivity >> 16) & 0xff;  
  xethru_send_buf[10] = (sensitivity >> 24) & 0xff;
  
  //Send the command
  send_command(11);
  
  // Get ACK response from radar
  get_ack();
}



// Set detection zone
void set_detection_zone(float zone_start, float zone_end) 
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_APPCOMMAND;
  xethru_send_buf[2] = XTS_SPCA_SET;
  xethru_send_buf[3] = XTS_ID_DETECTION_ZONE & 0xff;
  xethru_send_buf[4] = (XTS_ID_DETECTION_ZONE >> 8) & 0xff;
  xethru_send_buf[5] = (XTS_ID_DETECTION_ZONE >> 16) & 0xff;
  xethru_send_buf[6] = (XTS_ID_DETECTION_ZONE >> 24) & 0xff;
  
  // Copy the bytes of the floats to send buffer
  memcpy(xethru_send_buf+7, &zone_start, 4);
  memcpy(xethru_send_buf+11, &zone_end, 4);
  
  //Send the command
  send_command(15);
  
  // Get ACK response from radar
  get_ack();
}


  
// Run profile
void run_profile() 
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_MOD_SETMODE;
  xethru_send_buf[2] = XTS_SM_RUN;

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
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_DEBUG_LEVEL;
  xethru_send_buf[2] = 0x1f;

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
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_MOD_SETLEDCONTROL;
  xethru_send_buf[2] = control & 0xff;

  //Send the command
  send_command(3);
  
  // Get ACK response from radar
  get_ack();
}

// Load profile
void load_profile(uint32_t profile)
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_MOD_LOADAPP;
  xethru_send_buf[2] = profile & 0xff;
  xethru_send_buf[3] = (profile >> 8) & 0xff;
  xethru_send_buf[4] = (profile >> 16) & 0xff;  
  xethru_send_buf[5] = (profile >> 24) & 0xff;
  
  //Send the command
  send_command(6);
  
  // Get ACK response from radar
  get_ack();
}

void configure_noisemap(uint32_t code) 
{
  // xethru_send_buf[3] Configuration:
  //
  // Bit 0: FORCE INITIALIZE NOISEMAP ON RESET
  // Bit 1: ADAPTIVE NOISEMAP ON
  // Bit 2: USE DEFAULT NOISEMAP
  // 

  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_MOD_NOISEMAP;
  xethru_send_buf[2] = XTS_SPCN_SETCONTROL;
  xethru_send_buf[3] = code & 0xff; // 0x06: Use default noisemap and adaptive noisemap
  xethru_send_buf[4] = 0x00;
  xethru_send_buf[5] = 0x00;
  xethru_send_buf[6] = 0x00;

  //Send the command
  send_command(7);
  
  // Get ACK response from radar
  get_ack();
}


void enable_output_message(uint32_t message) 
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_OUTPUT;
  xethru_send_buf[2] = XTS_SPCO_SETCONTROL;
  xethru_send_buf[3] = message & 0xff;
  xethru_send_buf[4] = (message >> 8) & 0xff;
  xethru_send_buf[5] = (message >> 16) & 0xff;
  xethru_send_buf[6] = (message >> 24) & 0xff;
  xethru_send_buf[7] = XTID_OUTPUT_CONTROL_ENABLE & 0xff;
  xethru_send_buf[8] = (XTID_OUTPUT_CONTROL_ENABLE >> 8) & 0xff;
  xethru_send_buf[9] = (XTID_OUTPUT_CONTROL_ENABLE >> 16) & 0xff;
  xethru_send_buf[10] = (XTID_OUTPUT_CONTROL_ENABLE >> 24) & 0xff;

  //Send the command
  send_command(11);
  
  // Get ACK response from radar
  get_ack();
}


void disable_output_message(uint32_t message) 
{
  //Fill send buffer
  xethru_send_buf[0] = XT_START;
  xethru_send_buf[1] = XTS_SPC_OUTPUT;
  xethru_send_buf[2] = XTS_SPCO_SETCONTROL;
  xethru_send_buf[3] = message & 0xff;
  xethru_send_buf[4] = (message >> 8) & 0xff;
  xethru_send_buf[5] = (message >> 16) & 0xff;
  xethru_send_buf[6] = (message >> 24) & 0xff;
  xethru_send_buf[7] = XTID_OUTPUT_CONTROL_DISABLE & 0xff;
  xethru_send_buf[8] = (XTID_OUTPUT_CONTROL_DISABLE >> 8) & 0xff;
  xethru_send_buf[9] = (XTID_OUTPUT_CONTROL_DISABLE >> 16) & 0xff;
  xethru_send_buf[10] = (XTID_OUTPUT_CONTROL_DISABLE >> 24) & 0xff;

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

    if (xethru_recv_buf[1] != XTS_SPR_SYSTEM)
      continue;

    //uint32_t response_code = (uint32_t)xethru_recv_buf[2] | ((uint32_t)xethru_recv_buf[3] << 8) | ((uint32_t)xethru_recv_buf[4] << 16) | ((uint32_t)xethru_recv_buf[5] << 24);
    uint32_t response_code = *((uint32_t*)&xethru_recv_buf[2]);
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
  else if (xethru_recv_buf[1] != XTS_SPR_ACK) {  // Check response for ACK
    SerialDebug.println("Did not receive ACK!");
    errorPublish("ACK not received");
  }
}


/*
 * Adds CRC, Escaping and Stop byte to the
 * xethru_send_buf and sends it over the SerialRadar.
 */
void send_command(int len) 
{ 
  unsigned char crc = 0;
  
  // Calculate CRC
  for (int i = 0; i < len; i++)
    crc ^= xethru_send_buf[i];

  // Add CRC to send buffer
  xethru_send_buf[len] = crc;
  len++;
  
  // Go through send buffer and add escape characters where needed
  for (int i = 1; i < len; i++) {
    if (xethru_send_buf[i] == XT_START || xethru_send_buf[i] == XT_STOP || xethru_send_buf[i] == XT_ESCAPE)
    {
      // Shift following bytes one place up
      for (int u=len; u > i; u--)
        xethru_send_buf[u] = xethru_send_buf[u-1];

      // Add escape byte at old byte location
      xethru_send_buf[i] = XT_ESCAPE;

      // Increase length by one
      len++;
      
      // Increase counter so we don't process it again
      i++;
    }
  } 
  
  // Send data (including CRC) and XT_STOP
  SerialRadar.write(xethru_send_buf, len);
  SerialRadar.write(XT_STOP);

  // Print out sent data for debugging:
  SerialDebug.print("Sent: ");  
  for (int i = 0; i < len; i++) {
    SerialDebug.print(xethru_send_buf[i], HEX);  
    SerialDebug.print(" ");
  }
  SerialDebug.println(XT_STOP, HEX);
}
  
  
/* 
 * Receive data from radar module
 *  -Data is stored in the global array xethru_recv_buf[]
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
      xethru_recv_buf[0] = c;
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
      xethru_recv_buf[recv_len++] = c;
      break;  //Exit this loop 
    }

    if (recv_len >= RX_BUF_LENGTH) {
      //errorPublish("Buffer Overflow");
      SerialDebug.println("BUFFER OVERFLOW!");
      return -1;
    }
    
    // Fill response buffer, and increase counter
    xethru_recv_buf[recv_len++] = c;

    

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())
      delay(10);
  }

  // Print received data
  #if 0
  SerialDebug.print("Received: ");
  for (int i = 0; i < recv_len; i++) {
    SerialDebug.print(xethru_recv_buf[i], HEX);
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
  if (xethru_recv_buf[recv_len-1] != XT_STOP) {
    xethru_recv_buf[recv_len++] = XT_STOP; // If the message is missing the Stop Byte, just append it at the end.
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
    crc ^= xethru_recv_buf[i];
  
  // Check if calculated CRC matches the recieved
  if (crc == xethru_recv_buf[recv_len-2]) 
  {
    return recv_len;  // Return length of received data (without escape bytes) upon success
  }
  else 
  {
    //errorPublish("CRC mismatch");
    SerialDebug.print("CRC mismatch: ");
    SerialDebug.print(crc, HEX);
    SerialDebug.print(" != ");
    SerialDebug.println(xethru_recv_buf[recv_len-2], HEX);
    return -1; // Return -1 upon crc failure
  } 
}
