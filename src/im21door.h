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


//*************************function declarations*******************
void checkDoor();

#endif