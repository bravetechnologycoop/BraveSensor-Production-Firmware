/*
 *  Firmware config - tells us what type of firmware is being flashed
 * 
 * 
 */

#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

//#define PHOTON

#define DEBUG_LEVEL LOG_LEVEL_WARN
//v1.0 = 1, no 2 because no IM21 Argon
//v1.2 XeThru = 3, IM21 = 4
//v1.2.01 and v1.2.02: XeThru = 5, IM21 = 6
#define BRAVE_FIRMWARE_VERSION 5  

#define PRODUCT_ID_BETATEST 12858
#define PRODUCT_ID_PRODUCTION 12876


#define XETHRU_PARTICLE
//#define IM21_PARTICLE

#endif