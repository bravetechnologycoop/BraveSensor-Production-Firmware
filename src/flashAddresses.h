/*
 *
 * Brave firmware state machine for single Argon
 * written by Heidi Fedorak/Mario Cimet, Apr 2021
 * 
 * Edits for Xethru local state machine by James Seto
 * 
 *  Addresses for where in EEPROM certain variables are written.
 *  All except the flags are values that initialize to defaults 
 *  when firmware is first flashed, and can later be changed using
 *  console functions.
 *
*/

#ifndef FLASHADDRESSES_H
#define FLASHADDRESSES_H

//**********FLASH ADDRESSES***********************
//wifi
#define ADDR_SSIDS 0   		  		    	//sizeof = 320
#define ADDR_PWDS 320                           //sizeof = 320
#define ADDR_WIFI_DISCONNECT_LOG 640		    //sizeof = 4

//write originals flag
//flags are uint16_t so 2 bytes each
#define ADDR_INITIALIZE_SM_CONSTS_FLAG 642  //uint16_t = 2 bytes
#define ADDR_INITIALIZE_DOOR_ID_FLAG 644    //uint16_t = 2 bytes

//state machine constants
//all are uint32_t so 4 bytes each
#define ADDR_SLOW_THRESHOLD 648   	  		    	
#define ADDR_STATE1_MAX_TIME 652			        	
#define ADDR_STATE2_MAX_DURATION 656		
#define ADDR_STATE3_MAX_STILLNES_TIME 660

//im21 door sensor ID
//struct with three unsigned chars (uint8_t)
#define ADDR_IM21_DOORID 664	

//xethru
#define ADDR_XETHRU_LED 667		      	//sizeof = 4
#define ADDR_XETHRU_NOISEMAP 671      //sizeof = 4
#define ADDR_XETHRU_SENSITIVITY 675   //sizeof = 4
#define ADDR_XETHRU_MIN_DETECT 679    //sizeof = 4
#define ADDR_XETHRU_MAX_DETECT 683    //sizeof = 4

#define ADDR_INITIALIZE_XETHRU_CONSTS_FLAG 687 //sizeof = 2
//next available address is 691

#endif