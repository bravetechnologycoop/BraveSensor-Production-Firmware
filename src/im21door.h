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

#define HEARTBEAT8 0x08
#define HEARTBEATA 0x0A
#define LOWBATTERY 0X04
#define DOOROPEN 0x02
#define DOORCLOSED 0x00

#define DOOROPEN_PIN A0
#define DOORCLOSED_PIN A1
#define HEARTBEAT_PIN A2
#define LOWBATTERY_PIN A3


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
int publishViaBus(uint8_t doorData);


#endif