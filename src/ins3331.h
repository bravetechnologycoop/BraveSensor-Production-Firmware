/*
 * Project  addINScode
 * 
 * Description:  Integrating InnoSenT INS3331 firmware written by
 *      Sampath Satti with Argon firmware written primarily by Heidi
 *      Fedorak.  This will allow the master code package to connect 
 *      to XeThru or INS3331 depending on defines established in config.h
 * 
 * Author: Heidi Fedorak, Sampath Satti
 * Date:  Nov 2020
 * 
*/

#ifndef INS3331_H
#define INS3331_H

//***************************macro defines******************************

//SERIAL PORTS:
//These definitions work for Arduino Mega, but must be changed for other Arduinos.
//* Note: Using Serial as SerialRadar seems to give a few CRC errors. I'm not seeing this 
//  using Serial1, Serial2, or Serial3. Could probably be solved by changing baud rate)

#define SerialRadar Serial1    // Used for communication with the radar, Serial connection using TX,RX pins
#define SerialUSB Serial    // Used for printing debug information, Serial connection with (micro) USB

#define START_BYTE 0x11
#define START_DELIM 0xA2
#define MASTER_ADDRESS 0x01
#define RADAR_ADDRESS 0x80
#define END_DELIM 0x16

#define START_FC 0xE4
#define STOP_FC 0xEB


//***************************global variable declarations******************************



//***************************function declarations***************

//console functions

//setup functions
void ins3331Setup();
void radar_stop();
void radar_start();

//loop functions
void checkINS3331();
void cloudPublishINSData(String iValues, String qValues);
void usbSerialPrintINSData(String iValues, String qValues);
int twos_comp(int val, int bits);


#endif