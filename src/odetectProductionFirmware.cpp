/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/heidi/Documents/odetectProductionFirmware/src/odetectProductionFirmware.ino"
#include "xethru.h"
#include "wifi.h"



//*************************macros for setup/debugging that need to be altered during setup*************************

//#define PHOTON  //enables code for photon device


//*************************System/Startup messages for Particle API***********

//bootloader instructions to tell bootloader to run w/o wifi:
//enable system thread to ensure application loop is not 
//interrupted by system/network management functions
void setup();
void loop();
#line 16 "/home/heidi/Documents/odetectProductionFirmware/src/odetectProductionFirmware.ino"
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
  connectToWifi();

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
    connectToWifi();
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



