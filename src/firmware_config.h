/*
 *  Firmware config - tells us what type of firmware is being flashed
 * 
 * 
 */

#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

//#define PHOTON

#define DEBUG_LEVEL LOG_LEVEL_WARN

//for dual-argon products, radar is odd #s, door is evens:

//v1.0 = 1, no 2 because no IM21 Argon
//v1.2 XeThru = 3, IM21 = 4
//v1.2.01 XeThru = 5, IM21 = 6
//v1.2.02 XeThru = 7, IM21 = 8

//version 101 is covenant custom IM21 (should have been 102 to fit even-odd rule, whoops)

//beta testing versions start at 1000
//radar publish every 2s version: INS = 1001 IM21 = 1002
#define BRAVE_FIRMWARE_VERSION 1001

//12858 == Beta test units
//12876 == production units
#define BRAVE_PRODUCT_ID 12858

//#define XETHRU_PARTICLE
//#define IM21_PARTICLE
#define INS3331_PARTICLE

#endif