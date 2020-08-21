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

//*************************macro defines**********************************

//addresses of the start locations in EEPROM for the 5 SSID/password pairs
#define ADDRSSIDS 0
#define ADDRPWDS 320
//max string length of any SSID or password (including null char)
#define MAXLEN 64

#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB


//******************global variable declarations*******************
extern char mySSIDs[5][MAXLEN];
extern char myPasswords[5][MAXLEN];


//*************************function declarations*************************

void connectToWifi();
int setWifiSSID(String);
int setWifiPwd(String);   
void writeWifiToFlash();        
void readWifiFromFlash(); 
void initializeOriginalWifiCreds();
void wifiCredsSetup();

#endif