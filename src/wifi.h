/*
 * Project  updateWifiCredentialsRemotely
 * 
 * Description:  Allows wifi SSID and password to be changed via 
 * a Particle.function() call from the Particle Console
 * 
 * Author: Heidi Fedorak
 * Date:  July 2020
 * 
 */

#ifndef WIFI_H
#define WIFI_H

//*************************defines and global variables that need to be altered during setup*************************

#define USE_SERIAL  //when used, displays serial debugging messages

//#define WRITE_ORIGINALS  //writes original ssid/passwords to flash 


//set initial SSID/password pairs here
#define CLIENTSSID1 "ClientSSID1"
#define CLIENTSSID2 "Testbed"
#define CLIENTSSID3 "ClientSSID1"
#define CLIENTSSID4 "ClientSSID1"

#define CLIENTPWD1 "ClientPWD1"
#define CLIENTPWD2 "fireweed5"
#define CLIENTPWD3 "ClientPWD1"
#define CLIENTPWD4 "ClientPWD1"

//*************************global macro defines**********************************

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
#define ADDRSSIDS 0
#define ADDRPWDS 320
//max string length of any SSID or password (including null char)
#define MAXLEN 64

#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB


//******************global variable declarations*******************
extern int wifiBufferIndex;
extern char mySSIDs[5][MAXLEN];
extern char myPasswords[5][MAXLEN];

//*************************function declarations*************************

void connectToWifi(char array1[][MAXLEN], char array2[][MAXLEN]);
int setWifiSSID(String);
int setWifiPwd(String);   
void writeToFlash();        
void readFromFlash(); 

#endif