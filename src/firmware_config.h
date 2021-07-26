/*
 *  Firmware config - tells us what type of firmware is being flashed
 * 
 * 
 */

#ifndef FIRMWARE_CONFIG_H
#define FIRMWARE_CONFIG_H

//#define PHOTON

#define DEBUG_LEVEL LOG_LEVEL_WARN

// #define XETHRU_PARTICLE
#define IM21_PARTICLE

//beta test versions:
//no local version numbers yet, since not released to production.
//2 Argon INS version will probably be v1.3 in the repo, officially
//or whatever makes sense to align with the backend versioning system

//Particle console versions for the 2 Argon INS are:
//v--, INS = 1001, IM21 = 1002

//production versions:
//v1.0 = 1, no 2 because no IM21 Argon
//v1.2 XeThru = 3, IM21 = 4
//v1.2.01: XeThru = 5, IM21 = 6
//v1.2.02: Xethru = 7, IM21 = 8
//covenant house custom firmware is XeThru = 7, IM21 = 101
#define BRAVE_FIRMWARE_VERSION 102

#define BRAVE_PRODUCT_ID 12876  //12858 = betatest, 12876 = production

#endif