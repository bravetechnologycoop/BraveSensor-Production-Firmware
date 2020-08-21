#ifndef IM21DOOR_H
#define IM21DOOR_H

#define USE_SERIAL  //when used, displays serial debugging messages

#define DOORID_BYTE1 0x96
#define DOORID_BITE2 0x59
#define DOORid_BYTE3 0x27

#define SerialDebug Serial

void checkDoor();

#endif