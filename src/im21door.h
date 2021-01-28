/*
 * Project  IM21BLE
 * 
 * Description:  Program Particle Argon to receive door open
 *  and door closed advertising data from IM21 bluetooth low 
 *  energy door sensors.
 * 
 * Author: Heidi Fedorak
 * Date:  August 2020
 * 
 */

#ifndef IM21DOOR_H
#define IM21DOOR_H

//*************************global macro defines**********************************

//max length of deviceType and locationID strings
#define MAXLEN 64

//******************global variable declarations*******************
typedef struct IM21DoorID {
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
} IM21DoorID;

typedef struct doorData {
    unsigned char doorStatus;
    unsigned char controlByte;
} doorData;

//*************************function declarations*******************

//console functions
int setIM21DoorIDFromConsole(String command);
void writeIM21DoorIDToFlash(IM21DoorID);

//setup() functions
void setupIM21();
void printDeviceIdentifiersFromFlash();

//loop() functions
void checkIM21();
void logAndPublishDoorData(doorData previousDoorData, doorData currentDoorData);
void logAndPublishDoorWarning(doorData previousDoorData, doorData currentDoorData);

//common functions
//called from setupIM21() and console function:
IM21DoorID readIM21DoorIDFromFlash();


#endif