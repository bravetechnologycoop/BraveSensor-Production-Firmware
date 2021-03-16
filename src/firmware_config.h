/*
 *  Firmware config - tells us what type of firmware is being flashed
 * 
 * 
 */

#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

//#define PHOTON

#define DEBUG_LEVEL LOG_LEVEL_INFO
//v1.0 = 1, no 2 because no IM21 Argon
//v1.2 XeThru = 3, IM21 = 4
//v1.2.01 XeThru = 5, IM21 = 6
//version 101 is covenant custom IM21
#define BRAVE_FIRMWARE_VERSION 101  

//12858 == Beta test units
//12876 == production units
#define BRAVE_PRODUCT_ID 12876 //12858


//#define XETHRU_PARTICLE
#define IM21_PARTICLE

#endif