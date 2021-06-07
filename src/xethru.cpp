#include "Particle.h"
#include "flashAddresses.h"
#include "xethru.h"

//******************global variable initialization*******************

//these don't need to be global, I think, but they are used so often it would 
//take more time than I'm willing to spend fixing it. May revisit in the future.
unsigned char xethru_send_buf[TX_BUF_LENGTH];  // Buffer for sending data to radar.
unsigned char xethru_recv_buf[RX_BUF_LENGTH];  // Buffer for receiving data from radar.


//initialize constants to sensible default values
int xethru_led = XETHRU_LED_SETTING; 
int xethru_noisemap = XETHRU_NOISEMAP_SETTING;
int xethru_sensitivity = XETHRU_SENSITIVITY_SETTING; 
float xethru_min_detect = XETHRU_MIN_DETECT_SETTING;
float xethru_max_detect = XETHRU_MAX_DETECT_SETTING; 

// Multithread
os_queue_t xeThruQueue;

/***********************************************************called from setup() sub-functions************************************************************/

//called from setup(), contains xethru-specific setup
void setupXeThru(){

  pinMode(D6, INPUT_PULLUP); 
  //LEDSystemTheme theme; // Enable custom theme
  //theme.setColor(LED_SIGNAL_CLOUD_CONNECTED, 0x00000000); // Set LED_SIGNAL_NETWORK_ON to no color
  //theme.apply(); // Apply theme settings 

  // Set up serial communication
  SerialRadar.begin(115200);

  //load xethru flash:
  initializeXeThruConsts();

  xethru_reset();
  xethru_configuration();

  Log.warn("xeThruConfig read from flash during xethru setup:");
  Log.warn("led: %d, noisemap: %d, sensitivity: %d, min: %f, max: %f", 
            xethru_led, xethru_noisemap, xethru_sensitivity, xethru_min_detect, xethru_max_detect);

  // Wait for xethru to transmit response
  delay(3000);
  // Multithread:
  // Create a queue
  os_queue_create(&xeThruQueue, sizeof(SleepMessage), 128, 0);
	// Create the thread 
  new Thread("readXeThruThread", threadXeThruReader);
}


// Since there is no setup-firmware flash stage, the flash memory is set with default values here.
void initializeXeThruConsts(){

  uint16_t initializeXeThruFlag;

  //Argon flash memory is initialized to all F's (1's)
  EEPROM.get(ADDR_INITIALIZE_XETHRU_CONSTS_FLAG, initializeXeThruFlag);
  Log.info("Xethru constants flag is 0x%04X", initializeXeThruFlag);

  if(initializeXeThruFlag != INITIALIZE_XETHRU_CONSTS_FLAG){
    EEPROM.put(ADDR_XETHRU_LED, xethru_led);
    EEPROM.put(ADDR_XETHRU_NOISEMAP, xethru_noisemap);
    EEPROM.put(ADDR_XETHRU_SENSITIVITY, xethru_sensitivity);
    EEPROM.put(ADDR_XETHRU_MIN_DETECT, xethru_min_detect);
    EEPROM.put(ADDR_XETHRU_MAX_DETECT, xethru_max_detect);
    initializeXeThruFlag = INITIALIZE_XETHRU_CONSTS_FLAG;
    EEPROM.put(ADDR_INITIALIZE_XETHRU_CONSTS_FLAG, initializeXeThruFlag);
    Log.info("XeThru constants were written to flash on bootup.");
  }
  else{
    EEPROM.get(ADDR_XETHRU_LED, xethru_led);
    EEPROM.get(ADDR_XETHRU_NOISEMAP, xethru_noisemap);
    EEPROM.get(ADDR_XETHRU_SENSITIVITY, xethru_sensitivity);
    EEPROM.get(ADDR_XETHRU_MIN_DETECT, xethru_min_detect);
    EEPROM.get(ADDR_XETHRU_MAX_DETECT, xethru_max_detect);
    Log.info("XeThru constants were read from flash on bootup.");
  }


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
void xethru_configuration() {

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
  configure_noisemap();
  
  // Set LED control
  set_led_control(); // 0: OFF; 1: SIMPLE; 2: FULL

  // Set detection zone
  set_detection_zone(); // First variable = Lower limit, Second variable = Upper limit

  // Set sensitivity
  set_sensitivity();

  // Enable only the Sleep message, disable all others
  enable_output_message(XTS_ID_SLEEP_STATUS);
  disable_output_message(XTS_ID_RESP_STATUS);
  disable_output_message(XTS_ID_RESPIRATION_MOVINGLIST);
  disable_output_message(XTS_ID_RESPIRATION_DETECTIONLIST);

  // Run profile - after this the radar will start sending the sleep message we enabled above
  run_profile();
}


/********************************************************called from loop() or loop() sub-functions********************************************************/

/*
 * Checks queue if there is new data from the XeThru radar.
 * Returns a SleepMessage with new data and a timestamp at time of data pulled from queue
 * If no data in queue, returns a SleepMessage with all fields zero.
 */
SleepMessage checkXeThru(){
  SleepMessage dataToParse;
  static SleepMessage returnXeThruMessage = {0,0,0,0};

  // os_queue returns 0 on success
  if (os_queue_take(xeThruQueue, &dataToParse, 0, 0) == 0) {
    Log.info("Message taken from queue");
    returnXeThruMessage.movement_fast = dataToParse.movement_fast;
    returnXeThruMessage.movement_slow = dataToParse.movement_slow;
    returnXeThruMessage.state_code = dataToParse.state_code;
    returnXeThruMessage.timestamp = millis();
  } // end queue if
  return returnXeThruMessage;
} // end checkXeThru()


//*********************************threads***************************************

// Multithread 
void threadXeThruReader(void *param) {
  static unsigned char receiveBuffer[64];
  static int receiveBufferIndex;
  SleepMessage sleepMsg;
  static bool escFlag = false;
  static bool bufferFlag = false;
  static unsigned long lastUnavailable = millis();

  while(true){
    if (!SerialRadar.available()) lastUnavailable = millis();
    //Log.info("Looping through thread");
    if(SerialRadar.available() && millis() - lastUnavailable >= READ_RADAR_DELAY) {  // If this delay is removed the firmware will hard fault

      unsigned char c = SerialRadar.read();
      
      if (c == XT_ESCAPE) {
      // If it's an escape character next character in buffer is data and not special character:
        while (!SerialRadar.available());
        c = SerialRadar.read();
        escFlag = true;
      } 

      if(c == XT_START && !escFlag) receiveBufferIndex = 0;

      if(!bufferFlag) receiveBuffer[receiveBufferIndex] = c;
      receiveBufferIndex++;

      if (receiveBufferIndex >= RX_BUF_LENGTH) {
        //errorPublish("Buffer Overflow");
        Log.info("BUFFER OVERFLOW!");
        bufferFlag = true;
      } else {
        bufferFlag = false;
      }

      if(c == XT_STOP && !escFlag && !bufferFlag) {
        unsigned char crc = 0;
  
        // CRC is calculated without the crc itself and the stop byte, hence the -2 in the counter
        for (int i = 0; i < receiveBufferIndex-2; i++) 
          crc ^= receiveBuffer[i];

        // Check if calculated CRC matches the recieved
        if (crc != receiveBuffer[receiveBufferIndex-2]) 
        {
          Log.info("CRC mismatch: ");
          Log.info("crc, HEX: %u, %x", crc, HEX);
          Log.info(" != ");
          Log.info("receiveBuffer, HEX: %u, %x", receiveBuffer[receiveBufferIndex-2], HEX);
        } else {
          // Read message id
          uint32_t xts_id = *((uint32_t*)&receiveBuffer[2]);

          // Sleep message is the only relevant message containing movementFast, movementSlow
          if(xts_id == XTS_ID_SLEEP_STATUS) {
            sleepMsg.movement_slow = *((float*)&receiveBuffer[26]);
            sleepMsg.movement_fast = *((float*)&receiveBuffer[30]);
            sleepMsg.state_code = *((uint32_t*)&receiveBuffer[10]);

            //Log.info("Received sleep message: StateCode=%lu, MovementFast=%f, MovementSlow=%f", (unsigned long) sleepMsg.state_code, sleepMsg.movement_fast, sleepMsg.movement_slow);
            os_queue_put(xeThruQueue, (void *)&sleepMsg, 0, 0);
          } else {
            //Log.info("Received other message");
          }
        } // end crc if

      } // end stop if

      escFlag = false;

    } // end radar available if

  } // end thread while

} // end threadXeThruReader

/********************************************************sub-functions for all of the above****************************************************************/


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
void set_sensitivity() 
{
  uint32_t sensitivity = xethru_sensitivity;
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
void set_detection_zone() 
{
  float zone_start = xethru_min_detect;
  float zone_end = xethru_max_detect;
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
void set_led_control() 
{
  // For byte index 2
  // 0: OFF
  // 1: SIMPLE
  // 2: FULL
  uint32_t control = xethru_led;
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

void configure_noisemap() 
{
  // xethru_send_buf[3] Configuration:
  //
  // Bit 0: FORCE INITIALIZE NOISEMAP ON RESET
  // Bit 1: ADAPTIVE NOISEMAP ON
  // Bit 2: USE DEFAULT NOISEMAP
  // 
  uint32_t code = xethru_noisemap;
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
    Particle.publish("Xethru Error", message, PRIVATE);
}



// This method waits indefinitely for the XTS_SPRS_READY message from the radar
void wait_for_ready_message()
{
  Log.info("Waiting for XTS_SPRS_READY...");
  while (true) {
    if (receive_data() < 1)
      continue;

    if (xethru_recv_buf[1] != XTS_SPR_SYSTEM)
      continue;

    //uint32_t response_code = (uint32_t)xethru_recv_buf[2] | ((uint32_t)xethru_recv_buf[3] << 8) | ((uint32_t)xethru_recv_buf[4] << 16) | ((uint32_t)xethru_recv_buf[5] << 24);
    uint32_t response_code = *((uint32_t*)&xethru_recv_buf[2]);
    if (response_code == XTS_SPRS_READY) {
      Log.info("Received XTS_SPRS_READY!");
      return;
    }
    else if (response_code == XTS_SPRS_BOOTING)
      Log.info("Radar is booting...");
  }
}

// This method checks if an ACK was received from the radar
void get_ack()
{
  int len = receive_data();
  
  if (len == 0) {
    Log.info("No response from radar");
    //errorPublish("No reponse from radar");
  }
  else if (len < 0) {
    Log.info("Error in response from radar");
    //errorPublish("Error in response from radar");
  }
  else if (xethru_recv_buf[1] != XTS_SPR_ACK) {  // Check response for ACK
    Log.info("Did not receive ACK!");
    //errorPublish("ACK not received");
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
  Log.info("Sent: ");  
  for (int i = 0; i < len; i++) {
    Log.info("xethru_send_buf, HEX: %u, %x", xethru_send_buf[i], HEX);  
    Log.info(" ");
  }
  Log.info("XT_STOP, HEX: %x, %x", XT_STOP, HEX);
}
  
  
/* 
 * Used to receive data for XeThru setup.
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
      Log.info("BUFFER OVERFLOW!");
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
  Log.info("Received: ");
  for (int i = 0; i < recv_len; i++) {
    Log.info(xethru_recv_buf[i], HEX);
    Log.info(" ");
  }
  Log.info(" ");
  #endif
  
  // If nothing was received, return 0.
  if (recv_len==0) {
    //errorPublish("No message received");
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
    Log.info("CRC mismatch: ");
    Log.info("crc, HEX: %u, %x", crc, HEX);
    Log.info(" != ");
    Log.info("xethru_recv_buf, HEX: %u, %x", xethru_recv_buf[recv_len-2], HEX);
    return -1; // Return -1 upon crc failure
  } 
}

