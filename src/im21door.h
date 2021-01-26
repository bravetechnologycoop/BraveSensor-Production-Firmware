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

//******************global variable declarations*******************
typedef struct IM21DoorID {
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
} IM21DoorID;

typedef struct doorData {
    unsigned char doorStatus;
    unsigned char controlByte;
} doorData;

//*************************function declarations*******************

//console functions
int setIM21DoorIDFromConsole(String command);

//setup() functions
void setupIM21();

//loop() functions
void checkIM21();
void logAndPublishDoorData(doorData previousDoorData, doorData currentDoorData);

//common functions
//called from setupIM21() and console function:
void writeIM21DoorIDToFlash(IM21DoorID);
IM21DoorID readIM21DoorIDFromFlash();


#endif