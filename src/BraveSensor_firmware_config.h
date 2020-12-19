/*
 * Project:  Programmer input/setup 
 * 
 * Description:  
 * 
 *  This file contains all oDetect #define macros that 
 *  must be set by a human during oDetect setup, for example device ID's,
 *  Particle wifi credentials, etc etc.  
 *
 *  This file also contains macros for code execution options. For example 
 *  using debug messages displayed to a serial port or not, running Particle 
 *  device in manual mode or not, etc etc.
 * 
 * Directions:
 * 
 *  This file must be included in every .cpp file, and in the .ino file.
 *  Normally global header files would be included via gcc's AC_CONFIG_HEADERS
 *  or AH_HEADER macro. I, however, FLAT OUT REFUSE to screw with the Particle 
 *  makefiles! I don't want to be stuck with broken code and a broken toolchain
 *  for the next six months!!  grrrrrrr.....
 * 
 * Author: Heidi Fedorak
 * Date:  August 2020
 * 
 */
#ifndef ODETECT_CONFIG_H
#define ODETECT_CONFIG_H


//*************global settings*****************

//#define DOOR_PARTICLE
//#define XETHRU_PARTICLE
#define INS3331_PARTICLE
//#define PHOTON  //enables code for photon device
//#define SERIAL_DEBUG  //when used, displays serial debugging messages
//#define MANUAL_MODE  //lets code be flashed via USB serial without a pre-existing wifi connection. Good for debuging.


//**********FLASH ADDRESSES***********************

//wifi
#define ADDRSSIDS 0   			//sizeof = 320
#define ADDRPWDS 320			//sizeof = 320
#define ADDR_CONNECT_LOG 645	//sizeof = 4

//xethru
#define ADDR_XETHRUCONFIG 655	//sizeof = 20

//im21 door sensor
#define ADDR_IM21DOORID 700		//sizeof = 3

//*************Wifi Settings*******************

//writes original ssid/passwords to flash - see documentation for when and how to use this
//#define WRITE_ORIGINAL_WIFI  

//set initial SSID/password pairs here
//always use the "" string format
//Do not leave any SSID or password undefined: empty
//elements in char array will throw an error

#define CLIENTSSID0 "Testbed"
#define CLIENTSSID1 "Testbed"
#define CLIENTSSID2 "ClientSSID1"
#define CLIENTSSID3 "ClientSSID1"
//pasword needed to publish SSIDs in flash to the cloud
#define PASSWORD_FOR_SSIDS "testing"

#define CLIENTPWD0 "fireweed"
#define CLIENTPWD1 "fireweed5"
#define CLIENTPWD2 "ClientPWD1"
#define CLIENTPWD3 "ClientPWD1"
//pasword needed to publish passwords in flash to the cloud
#define PASSWORD_FOR_PASSWORDS "testing"

//*************Bluetooth Door Sensor Settings***********

//#define WRITE_ORIGINAL_DOORID

//door ID for each individual IM21 broadcast in advertising data is 3 bytes
//found on sticker on button: bottom row of numbers/letters, take the first
//three bytes listed and enter them in reverse order. For example if bottom 
//row of numbers/letters on sticker is 1a2b3c45, door ID will be 3c 2b 1a. 
#define DOORID_BYTE1 0x96
#define DOORID_BYTE2 0x59
#define DOORID_BYTE3 0x27

//************XeThru Settings**************

//#define WRITE_ORIGINAL_XETHRU

#define LOCATIONID "Heidis House"
#define DEVICEID "HeidiTest"
#define DEVICETYPE "XeThru"

// XeThru configuration variables
#define XETHRU_LED_SETTING 0
#define XETHRU_NOISEMAP_SETTING 0
#define XETHRU_SENSITIVITY_SETTING 5
#define XETHRU_MIN_DETECT_SETTING 0.5
#define XETHRU_MAX_DETECT_SETTING 4

//************INS3331 Settings**************
#define INS_LOCATIONID "Heidis House"
#define INS_DEVICEID "HeidiTest"
#define INS_DEVICETYPE "INS3331"


#endif
