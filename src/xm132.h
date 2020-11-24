/*
 * 
 * XeThru libraries:
 * Written by: �yvind Nydal Dahl
 * Company: XeThru / Novelda
 * July 2018
 * 
 * Original code to read data from XeThru and transmit to 
 * Particle cloud written by Sampath Satti, Wayne Ng, and Sajan Rajdev.
 * 
 * XeThru code upgraded to be scaleable by Heidi Fedorak:
 *    -established separate .h and .cpp files for XeThru code
 *    -redefined global variables appropriately
 *    -established odetect_config.h file for all global defines
 *    -removed delay(1000) and adapted publishXethruData() to control
 *     rate of messages being published to the cloud
 * 
 * 
 */

#ifndef XM132_H
#define XM132_H

//***************************macro defines******************************


#define SerialRadar Serial1    // Used for communication with the radar, Serial connection using TX,RX pins
#define SerialDebug Serial    // Used for printing debug information, Serial connection with (micro) USB

#define MODE_SELECTION_REGISTER 0x02
#define POWER_BINS_SERVICE 0x01
#define ENVELOPE_SERVICE 0x02
#define SPARSE_SERVICE 0x04
#define DISTANCE_SERVICE 0x200
#define PRESENCE_SERVICE 0x400

#define MAIN_CONTROL_REGISTER 0x03
#define STOP_SERVICE 0x00
#define START_SERVICE 0x03

#define STREAMING_CONTROL_REGISTER 0x05
#define UART_OFF 0x00
#define UART_ON 0x01


//***************************global variable declarations******************************


//***************************function declarations***************

//console functions

//loop() functions and sub-functions:

//setup() functions and sub-functions:
void xm132Setup();
void writeToXM132(unsigned char address, unsigned char value[4]);



#endif