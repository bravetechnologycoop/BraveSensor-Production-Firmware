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

#ifndef PARALLELBUS_H
#define PARALLELBUS_H

//*************************global macro defines**********************************

#define DOOROPEN_PIN A0
#define DOORCLOSED_PIN A1
#define HEARTBEAT_PIN A2
#define LOWBATTERY_PIN A3


//******************global variable declarations*******************


//*************************function declarations*******************

//console functions

//setup() functions
void parallelBusSetup();

//isr functions
void doorOpenISR();
void doorClosedISR();
void heartbeatISR();
void lowBatteryISR();

//loop functions
void checkParallelBus();

#endif