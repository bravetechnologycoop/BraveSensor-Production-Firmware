/*
 * Project:  Programmer input/setup
 *
 * Description:
 *
 *  This file contains all BraveSensor #define macros that
 *  must be set by a human during BraveSensor setup, for example device ID's,
 *  Particle wifi credentials, etc etc.
 *
 * Directions:
 *  
 *  See documentation for what these settings mean.  Important to note, this 
 *  file should not be tracked by git since it contains sensitive client passwords.
 *  This file should be backed up elsewhere, privately.
 *
 *
 * Author: Heidi Fedorak
 * Date:  Jan 2021
 *
 */
#ifndef SETUP_FIRMWARE_H
#define SETUP_FIRMWARE_H

//*************global settings*****************

//delcare what log level you want for setup firmware:

#define DEBUG_LEVEL LOG_LEVEL_WARN

//declare what type of device you are setting up. Uncomment the 
//appropriate define, do not set it to anything

#define XETHRU_PARTICLE
//#define IM21_PARTICLE
//#define INS3331_PARTICLE
//#define XM132_PARTICLE

//***********Device Info**************************

//pay attention to data type (string vs int) and size constraints:

//always use the "" string format for strings
//string max length is 63 chars + null char = 64 chars
//do not leave anything undefined, empty char arrays will throw error
//device ID must be an int

#define LOCATIONID "HeidiTesting"           
#define DEVICETYPE "XeThruArgon"     
#define DEVICEID 1  //int             


//*************Wifi Settings*******************

//see documentation for more info on when and how to use this

//pay attention to the following size and type constraints:

//always use the "" string format
//Do not leave any SSID or password undefined: empty elements in char array will throw an error
//Strings are limited to 62 characters each: 1st char reserved for array index (see changeSSID cloud function)
//and last char is for null char at the end of the string.

#define CLIENTSSID0 "Client"
#define CLIENTSSID1 "Client"
#define CLIENTSSID2 "ssid"
#define CLIENTSSID3 "ssid"


#define CLIENTPWD0 "password123"
#define CLIENTPWD1 "password1234"
#define CLIENTPWD2 "password"
#define CLIENTPWD3 "password"

//************XeThru Settings**************

// make sure you enter a setting of the appropriate type (float vs int)
#define XETHRU_LED_SETTING 0            //int
#define XETHRU_NOISEMAP_SETTING 0       //int
#define XETHRU_SENSITIVITY_SETTING 5    //int
#define XETHRU_MIN_DETECT_SETTING 0.5   //float
#define XETHRU_MAX_DETECT_SETTING 4.0   //float

#endif
