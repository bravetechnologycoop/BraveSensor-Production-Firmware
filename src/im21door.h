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

#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB


//******************global variable declarations*******************
typedef struct IM21DoorIDStruct {
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
} IM21DoorIDStruct;

//*************************function declarations*******************

//console functions
int doorSensorIDFromConsole(String command);

//setup() functions
void doorSensorSetup();
//called from doorSensorSetup() and doorSensorIDFromConsole():
void initOriginals(IM21DoorIDStruct* structToInitialize);
void writeDoorIDToFlash(IM21DoorIDStruct* structPtr);
IM21DoorIDStruct readDoorIDFromFlash();

//loop() functions
int checkDoor();



#endif