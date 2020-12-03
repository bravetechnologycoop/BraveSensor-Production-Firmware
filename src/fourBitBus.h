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

#ifndef FOURBITBUS_H
#define FOURBITBUS_H

//*************************global macro defines**********************************

#define DOOROPEN_PIN A0
#define DOORCLOSED_PIN A1
#define HEARTBEAT_PIN A2
#define LOWBATTERY_PIN A3


//******************global variable declarations*******************

uint8_t doorOpen = 0;
uint8_t doorClosed = 0;
uint8_t heartbeat = 0;
uint8_t lowBattery = 0;


//*************************function declarations*******************

//console functions

//setup() functions
void fourBitBusSetup();

//isr functions
void doorOpenISR();
void doorClosedISR();
void heartbeatISR();
void lowBatteryISR();

//loop functions


#endif