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

//max string length of any SSID or password (including null char)
#define MAXLEN 64

//******************global variable declarations*******************
//variables used in setup() and loop() have to be global
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
void writeWifiLogToFlash(int wifiLogCount);
int readWifiLogFromFlash();
int wifiLog(String logCommand);

#endif