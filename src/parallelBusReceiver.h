/*
 * Project  IM21BLE
 * 
 * Description:  Program Particle Argon to receive door open
 *  and door closed advertising data from IM21 bluetooth low 
 *  energy door sensors, via a parallel bus connected to GPIO
 *  pins on two respective particle Argons.
 * 
 *  Radar Argon will always be the bus receiver, and Door Argon
 *  will always be the bus transmitter.
 * 
 * Author: Heidi Fedorak
 * Date:  August 2020
 * 
 */

#ifndef PARALLELBUSRECEIVER_H
#define PARALLELBUSRECEIVER_H

//*************************global macro defines**********************************

#define DOOROPEN_PIN A0
#define DOORCLOSED_PIN A1
#define HEARTBEAT_PIN A2
#define LOWBATTERY_PIN A3




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