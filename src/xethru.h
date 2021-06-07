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
 *    -removed delay(1000) and adapted publishXethruData() to control
 *     rate of messages being published to the cloud
 * 
 * Edits for Xethru local state machine by James Seto
 *    -defined global constants for xethru settings
 *    -added multithread to read sleep messages from XeThru and push to queue
 *    -changed checkXethru to pop data from queue and return data
 *    -refactored console function into consoleFunctons.cpp
 */

#ifndef XETHRU_H
#define XETHRU_H

//***************************macro defines******************************



//SERIAL PORTS:
//These definitions work for Arduino Mega, but must be changed for other Arduinos.
//* Note: Using Serial as SerialRadar seems to give a few CRC errors. I'm not seeing this 
//  using Serial1, Serial2, or Serial3. Could probably be solved by changing baud rate)

#define SerialRadar Serial1    // Used for communication with the radar, Serial connection using TX,RX pins
#define MAXLEN 64  //max length of locationID and deviceType stored in flash

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

// Reasonable default values, to be stored in EEPROM if consts are not initialized
#define XETHRU_LED_SETTING 0            //int
#define XETHRU_NOISEMAP_SETTING 0       //int
#define XETHRU_SENSITIVITY_SETTING 5    //int
#define XETHRU_MIN_DETECT_SETTING 0.5   //float
#define XETHRU_MAX_DETECT_SETTING 4.0   //float

//ascii table goes up to 7F, so pick something greater than that 
//which is also unlikely to be part of a door ID or a threshold/timer const
#define INITIALIZE_XETHRU_CONSTS_FLAG 0x8888

#define READ_RADAR_DELAY 500 // delay between radar reads

//***************************global variable declarations******************************

// Xethru settings, initialized in setup and modified with console function
extern int xethru_led;
extern int xethru_noisemap;
extern int xethru_sensitivity;
extern float xethru_min_detect;
extern float xethru_max_detect;

// Struct to hold sleep message from radar
typedef struct SleepMessage {
  uint32_t state_code;
  float movement_slow;
  float movement_fast;
  unsigned long timestamp;
} SleepMessage;

//***************************function declarations***************

//loop() functions and sub-functions:
SleepMessage checkXeThru(void);

//threads
void threadXeThruReader(void *param);

//setup() functions and sub-functions:
void setupXeThru();
void xethru_reset();  //this one also called from console function
void initializeXeThruConsts(); // replaces xethru_configuration
void xethru_configuration();

//called from xethru_configuration():
void wait_for_ready_message();  //this calls receive_data()
void stop_module();
void set_debug_level();
void load_profile(uint32_t profile);
void configure_noisemap();
void set_led_control();
void set_detection_zone();
void set_sensitivity();
void enable_output_message(uint32_t message);
void disable_output_message(uint32_t message);
void run_profile();

//called from most of the xethru_configuration() sub-functions:
void send_command(int len);
void get_ack();  //this calls receive_data()

//called from get_ack() and receive_data():
void errorPublish(String message);

//called from get_respiration_data(), wait_for_ready_message(), get_ack()
int receive_data();

#endif