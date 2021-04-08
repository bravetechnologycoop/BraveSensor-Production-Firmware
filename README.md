# SensorProductionFirmware - out of date...

This Repository approximately follows the gitflow workflow method, see the [tutorial](https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow)  It does not use a Develop branch, however; this repo only contains the main (master) branch, Release branch, and feature branches.

Tags denote the different releases pushed to the master branch.  

As of Apr 7/21, the different product firmware versions in this repo are:

2 Argon + XeThru - in the main (master) branch.
2 Argon + INS - in the Release branch.
1 Boron + INS - in the Local-State-Machine branch.


## Table of Contents

1. [Table of Contents](#table-of-contents)
2. [Two Argon and XeThru Firmware](#two-argon-and-xethru-firmware)
      - [Firmware Settings and Config](#firmware-settings-and-config)
        - [DOOR_PARTICLE](#door_particle)
        - [XETHRU_PARTICLE](#xethru_particle)
        - [DEBUG_LEVEL](#debug_level)
        - [WRITE_ORIGINAL_XX](#write_original_xx)
        - [PHOTON](#photon)
        - [MANUAL MODE](#manual-mode)
    - [Flash Addresses](#flash-addresses)
    - [Wifi Settings](#wifi-settings)
        - [Retrieving Contents of Flash Memory](#retrieving-contents-of-flash-memory)
        - [Setting Client Wifi Credentials](#setting-client-wifi-credentials)
    - [Door Sensor Settings](#im21-bluetooth-door-sensor)
    - [Xethru Settings](#xethru-settings)
        - [locationID, deviceID, devicetype](#locationid-deviceid-devicetype)
        - [Configuration Variables](#configuration-variables)
3. [Console Function Instructions](#console-function-instructions)
    - [ChangeSSID(String)](#changessid(string))
    - [ChangePwd(String)](#changepwd(string))
    - [getWifiLog(String)](#getwifilog(string))
    - [doorSensorID(String)](#doorsensorid(string))
    - [xethruConfigVals(String)](#xethruconfigvals(string))
4. [Published Events](#published-events)
    - [XeThru](#xethru)
    - [XeThru Error](#xethru-error)
    - [Current Xethru Config Settings](#current-xethru-config-settings)
    - [Current SSIDs](#current-ssids)
    - [WiFi Disconnect Warning](#wifidisconnectwarning)
    - [Door](#door)
    - [Door Warning](#door-warning)
    - [Current Door Sensor ID](#current-door-sensor-id)
    - [spark/device/diagnostics/update](#spark)
5. [2 Argon and INS Firmware](#2argon-ins)
6. [Webhook Templates](#webhook-templates)
    - [XeThru Template](#xethru-template)
    - [IM21 Template](#im21-template)
7. [Firmware State Machine](#firmware-state-machine)

## Two Argon and XeThru Firmware

As of Apr 7/21, this firmware is in the main (master) branch, and is in production on the devices at client sites.

### Firmware Settings and Config

Setup firmware must be flashed to these devices before the production firmware can be flashed to them.  Setup firmware is found in the Setup-Firmware branch.  Step-by-step instructions on how to flash this and the production firmware are shared with devs internally.

### Global Settings

Global settings control what portions of the code will be compiled for flashing.

#### DOOR_PARTICLE

A “Door Particle” connects to the IM21 door sensor via bluetooth low energy, and relays door open/door closed events to the cloud.  Defining DOOR_PARTICLE will enable the IM21 door sensor code for compilation/flashing to a Particle.

Do not define XETHRU_PARTICLE at the same time unless you want both the door sensor and the Xethru breath sensor to be operated by the same Particle.  This is not advisable as door open/closed events will be dropped.

#### XETHRU_PARTICLE

A “Xethru Particle” connects to the Xethru breath sensor and relays breath data to the cloud.  Defining XETHRU_PARTICLE will enable the xethru sensor firmware for compilation/flashing to a Particle.

Do not define DOOR_PARTICLE at the same time unless you want both the door sensor and the Xethru breath sensor to be operated by the same Particle.  This is not advisable as door open/closed events will be dropped.

#### DEBUG_LEVEL

Define this as one of Particle's supported log levels, found [here](https://docs.particle.io/reference/device-os/firmware/argon/#logging-levels).  All debugging within ODetect firmware is done at the "info" log level, to see them displayed use LOG_LEVEL_INFO or higher.  Example:

``` C++
#define DEBUG_LEVEL LOG_LEVEL_INFO
```

If you are connect to the Particle device via USB, these logs can be read by opening a Particle [command line interface](https://docs.particle.io/reference/developer-tools/cli/) and using the command "particle serial monitor --follow".

#### WRITE_ORIGINAL_xx

Currently there is a write original macro for wifi, door ID, and Xethru.  These are defined when you are setting up a Particle that is not new out of box, or you need to rewrite original settings to flash memory for any reason (usually debugging).

Original settings are any settings defined in the odetect_config.h file, such as the wifi credentials and the door sensor’s device ID.  They are referred to as original settings since what is defined in odetect_config.h can be overwritten in the future by the Particle console functions.  

**NEW OUT OF BOX:** Each Particle device ships with 0xFF written to all bytes of flash memory, see [this link](https://docs.particle.io/reference/device-os/firmware/argon/#get-). In setup() the firmware checks whether the first two bytes of memory contain 0xFFFF.  If it does, the Particle is considered new out of box and writes the original settings to flash memory.  When the Particle reboots in the future, it will see something other than 0xFFFF is written to memory.  It will then read what is there instead of re-writing the original settings.

**NOT NEW OUT OF BOX:**  If your Particle is not new out of box, there is a chance previous code has written data to flash.  In this case, the firmware in setup() will likely see something other than 0xFFFF written to memory and try to read whatever garbage was stored previously as the original settings.  This will probably break everything.  To prevent this and ensure original settings are written, you must define all three WRITE_ORIGINAL_xxxxx.

**IMPORTANT:**  Once you have flashed your code with any of the WRITE_ORIGINAL_xxxx defines enabled, you must disable the define(s) and re-flash.  If you forget this step, whenever the Particle experiences a power cycle it will continue to write original settings to flash and overwrite any newer information that may have been set by the Particle console functions.

#### PHOTON

Define this if your device is a photon, so photon-specific code can be compiled/flashed. The default device is currently an Argon.  (Presently all this macro does is turn on the Photon’s external wifi antenna, all other code is identical to the Argon.)

#### MANUAL_MODE

Define this to enable the Particle to run in Manual Mode.  This allows code to be flashed via USB Serial without a pre-existing wifi connection, and for the programmer to control connection to wifi and the cloud manually.  Good for debugging.

See [particle system modes](https://docs.particle.io/reference/device-os/firmware/argon/#system-modes).

### Flash Addresses

These defines set the flash memory addresses of variables backed up to flash memory.  All numbers are in bytes.

- ADDRSSIDS:  Flash memory address where char array of SSIDs is stored. Size: 320 bytes.
- ADDRPWDS:  Flash memory address where char array of passwords is stored.  Size: 320 bytes.
- ADDR_CONNECT_LOG:  Flash memory address where minimalist wifi connectivity log, aka an int, is stored.  Size: 4 bytes.  This int contains the number of times connectToWifi() is called.  connectToWifi() is only called when the Particle has lost connection to wifi and is attempting to reconnect.  This int acts as a minimalist log to determine whether a particular Particle is having trouble remaining connected to wifi.
- ADDR_XETHRUCONFIG:  Flash memory address where struct of Xethru configuration settings is stored.  Size: 20 bytes.
- ADDR_IM21DOORID:  Flash memory address where IM21 door sensor device ID is stored.  Size: 3 bytes.

### Wifi Settings

In this section you define client wifi credentials, and two internal Brave passwords that can be entered to the Particle console functions to publish credentials in flash memory.  

**Always use `“”` string quotes to indicate string format.  Do not leave any define blank in this section or the code will break.**

#### Retrieving Contents of Flash Memory

If the internal Brave passwords are entered into the console functions, the wifi credentials currently stored in flash memory will be published to the cloud.  You may set the passwords as follows:

- PASSWORD_FOR_SSIDS: “console_function_password_to_echo_SSIDs”
- PASSWORD_FOR_PASSWORDS: “console_function_password_to_echo_passwords”

#### Setting Client Wifi Credentials

Define the CLIENTSSIDX and CLIENTPWDX macros to the client wifi credentials, where X is the index number of the wifi/password pair.

SSID/password pairs share a common index number.  For example, CLIENTPWD0 contains the password corresponding to CLIENTSSID0, CLIENTPWD1 is the password for CLIENTSSID1, and so on.

We are limited to 62 characters per SSID or password:  the WEP/WEP2 standards for SSID and password length is 64 characters.  In this firmware we reserve one character for null character and one character for the index number (see console function changeSSID() below).  It is not necessary to include the null character in the string.

You may have up to four unique SSIDs, but the SSIDs are not required to be unique. For example, if you have three different passwords for one SSID, and a second SSID with its own password, you may define them as:

``` C++
#define CLIENTSSID0 "ClientSSID1"
#define CLIENTSSID1 "ClientSSID1"
#define CLIENTSSID2 "ClientSSID1"
#define CLIENTSSID3 "ClientSSID2"

#define CLIENTPWD0 "password1_for_SSID1"
#define CLIENTPWD1 "password2_for_SSID1"
#define CLIENTPWD2 "password3_for_SSID1"
#define CLIENTPWD3 "password_for_SSID2"
```

We are limited to 5 SSID/password pairs by the functionality of WiFi.setCredentials() in the Particle API.  The last set of credentials is reserved for the Brave diagnostics network, so we have the option of setting up to four different sets of wifi credentials for the customer.  It is important to remember these credentials are also stored in flash memory and can be overwritten in the future by the Particle console functions.  The only way originals can be restored to flash memory after being overwritten is by defining WRITE_ORIGINAL_WIFI.  See the WRITE_ORIGINAL_xx section above for more details.

### Door Sensor Settings 

The door sensor device ID for each individual IM21 is broadcast by the door sensor in its advertising data.  This door ID is 3 bytes, which we define in this section.  

When the firmware scans nearby bluetooth low energy devices, it finds the advertising data containing this door ID, extracts the door status (open or closed), and publishes that to the cloud.

The IM21 door sensors each have a sticker on them with their door IDs.  On the bottom row of numbers and letters, take the first three bytes listed and define them in reverse order.  For example, if the bottom row of numbers and letters on the sticker is 1a2b3c45, the door ID will be 3c:2b:1a.  This door ID is defined like:

```C++
#define DOORID_BYTE1 0x3c
#define DOORID_BYTE2 0x2b
#define DOORID_BYTE3 0x1a
```

### XeThru Settings

#### locationID, deviceID, devicetype

Each of these must be initialised to a string array containing the correct information for your particular install.

**Always use `“”` string quotes to indicate string format.  Do not leave any define blank or the code will break.**

LocationID is a UID corresponding to each bathroom and must batch with the row entry on the backend locations table. Current locationID entries look thus “REACH_1”, “REACH_3”, “EastsideWorks”.

DeviceID is currently redundant and will be removed in future versions of the firmware. DO NOT USE. Intialize DeviceID with “42” so as to not have any issues with null string arrays.

Device type is “XeThru”

#### CONFIGURATION VARIABLES

Led, noisemap, and sensitivity settings are ints, so defining a decimal number will break the code.  Min detect and max detect are floats, so decimal numbers can be used there.  An example is below:

```C++
#define XETHRU_LED_SETTING 0
#define XETHRU_NOISEMAP_SETTING 0
#define XETHRU_SENSITIVITY_SETTING 5
#define XETHRU_MIN_DETECT_SETTING 0.5
#define XETHRU_MAX_DETECT_SETTING 4.0
```

**LED:**

- 0: OFF
- 1: SIMPLE
- 2: FULL

**NoiseMap:**
- Bit 0: FORCE INITIALIZE NOISEMAP ON RESET
- Bit 1: ADAPTIVE NOISEMAP ON
- Bit 2: USE DEFAULT NOISEMAP

**Sensitivity:**
Scale goes from 0 to 10.

**Max and Min Detect:**
Set the maximum and minimum range (in metres) for detecting movement.

## Console Function Instructions

All console functions accept only a single arduino String as input, and return an int as output.

### changeSSID(String)

**Description:**

Writes new SSIDs to flash memory on the Particle devices, or publishes current contents of flash to cloud.  If wifi is disconnected the Particle will read these from flash and attempt to connect to them.  

SSID/password pairs share a common index number.  For example, CLIENTPWD0 contains the password corresponding to CLIENTSSID0, CLIENTPWD1 is the password for CLIENTSSID1, and so on.

We are limited to 62 characters per SSID or password:  the WEP/WEP2 standards for SSID and password length is 64 characters.  In this firmware we reserve one character for null character and one character for the index number (see console function changeSSID() below).  It is not necessary to include the null character in the string.

**Argument(s):**

All console functions only accept a single arduino String.  It is not necessary to enter the string with surrounding `“”` quotes.  The different strings this function accepts are:

1. A string whose first character is an index number 0 - 3 indicating which SSID/password pair this SSID belongs to, and the remaining characters are the SSID itself.  For example, to place ClientSSID in the 0th element of the firmware’s five-SSID array, enter 0ClientSSID
2. A string containing a password defined in odetect_config.h.  If this password is entered, the SSIDs currently stored in flash memory will be published to the cloud.

**Additional Information:**

You may have up to four unique SSIDs, but the SSIDs are not required to be unique. For example, if you have three different passwords for one SSID, and a second SSID with its own password, you may define them as:

- 0ClientSSID0  and  0password_for_SSID0
- 1ClientSSID0  and  1second_password_for_SSID0
- 2ClientSSID0  and  2third_password_for_SSID0
- 3ClientSSID1  and 3password_for_SSID1

The fifth SSID/password pair is reserved for the internal Brave diagnostics network.  This network is hardcoded into the firmware.  The console functions have read-only access to this network.  It cannot be changed remotely.

**Return(s):**

- 2: when correct password is entered and SSIDs are published to the cloud
- 0 - 3: the index the SSID was stored in, if it was stored successfully
- -1: incorrect password to publish SSIDs entered, or SSID not stored correctly

### changePwd(String)

**Description:**

Writes new passwords to flash memory on the Particle devices, or publishes current contents of flash to cloud.  If wifi is disconnected the Particle will read these from flash and attempt to connect to them.  

SSID/password pairs share a common index number.  For example, CLIENTPWD0 contains the password corresponding to CLIENTSSID0, CLIENTPWD1 is the password for CLIENTSSID1, and so on.

We are limited to 62 characters per SSID or password:  the WEP/WEP2 standards for SSID and password length is 64 characters.  In this firmware we reserve one character for null character and one character for the index number (see console function changeSSID() below).  It is not necessary to include the null character in the string.

**Argument(s):**

All console functions only accept a single arduino String.  It is not necessary to enter the string with surrounding `“”` quotes.  The different strings this function accepts are:

1. A string whose first character is an index number 0 - 3 indicating which SSID/password pair this password belongs to, and the remaining characters are the password itself.  For example, if ClientSSID is in the 0th element of the firmware’s five-SSID array, enter its password as 0password_for_ClientSSID0  
2. A string containing a password defined in odetect_config.h.  If this password is entered, the passwords currently stored in flash memory will be published to the cloud.

**Additional Information:**

You may have up to four unique SSIDs, but the SSIDs are not required to be unique. For example, if you have three different passwords for one SSID, and a second SSID with its own password, you may define them as:

- 0ClientSSID0  and  0password_for_SSID0
- 1ClientSSID0  and  1second_password_for_SSID0
- 2ClientSSID0  and  2third_password_for_SSID0
- 3ClientSSID1  and  3password_for_SSID1

The fifth SSID/password pair is reserved for the internal Brave diagnostics network.  This network is hardcoded into the firmware.  The console functions have read-only access to this network.  It cannot be changed remotely.

**Return(s):**

- -2: when correct password is entered and client passwords are published to the cloud
- 0 - 3: the index the password was stored in, if it was stored successfully
- -1: incorrect password to access client passwords entered, or client password not stored correctly

### getWifiLog(String)

**Description:**

 This function publishes the wifi log to the cloud, or resets the log to 0.  The wifi log is a single int that increments every time the Particle loses connection to wifi and firmware function connectToWifi() must be called.  It is a minimalist log to determine if a particular Particle has difficulty maintaining a wifi connection.

**Argument(s):**

All console functions only accept a single arduino String.  It is not necessary to enter the string with surrounding `“”` quotes.  The different strings this function accepts are:

1. e - echos, aka publish to cloud, the wifi log’s current int
2. c - clears the wifi log by setting the int = 0

**Return(s):**

- Wifi log number if e or c is received and parsed correctly
- -1 - when bad input is received

### doorSensorID(String)

**Description:**

Sets a new door ID for Particle to connect to, or publishes current door ID to cloud.  If new door ID is set, reconnection to new door sensor should occur instantly. Door ID is the three byte device ID for the IM21 bluetooth low energy sensor that the Particle is currently connected to.

**Additional Information:**

This console function appears only for Particles that are connected to an IM21 sensor.  Particles connected to and running firmware for the Xethru breath sensor will not have this console function.

When the firmware scans nearby bluetooth low energy devices, it finds the advertising data containing the IM21’s door ID, extracts the door status (open or closed), and publishes that to the cloud.

The IM21 door sensors each have a sticker on them with their door IDs.  On the bottom row of numbers and letters, take the first three bytes listed and define them in reverse order.  For example, if the bottom row of numbers and letters on the sticker is 1a2b3c45, the door ID will be 3c:2b:1a.

**Argument(s):**

All console functions only accept a single arduino String.  It is not necessary to enter the string with surrounding `“”` quotes.  The different strings this function accepts are:

1. Three byte door ID separated by commas, for example 3C,2B,1A  See Additional Information section above for where to locate an IM21 door sensor’s door ID.  
2. e - echos, aka publish to cloud, the door ID the Particle is currently connected to

**Return(s):**

- 0 - if door ID parsed and written to flash
- 1 - if door ID echoed to the cloud
- -1 - if bad input was received and door ID was neither parsed or echoed to the cloud

### xethruConfigVals(String)

**Description:**

Writes new Xethru configuration settings to Particle and restarts the Xethru sensor.  New settings should take effect immediately after Xethru restart.

Configuration settings are led, noisemap, sensitivity, min_detect, and max_detect.  Led, noisemap, and sensitivity are ints, min_detect and max_detect are floats.

**Argument(s):**

All console functions only accept a single arduino String.  It is not necessary to enter the string with surrounding `“”` quotes.  The different string(s) this function accepts are:

1. String containing led, noisemap, sensitivity, min_detect, and max_detect settings, in that order, separated by commas.  Led, noisemap, and sensitivity are ints so entering a decimal number will cause truncation and possibly an error.
Min_detect and max_detect are floats.
**Sample input:**   0, 0, 5, 0.5, 4.0
2. e - echos, aka publish to cloud, the current Xethru configuration settings

**Return(s):**

- 0 - if new settings parsed and written to flash, and xethru restarted
- 1 - if current settings are echoed to the cloud
- -1 - if bad input was received and settings were neither parsed nor echoed to the cloud

## Published Events

Information about the events the Particle firmware currently publishes to the cloud.

### “Xethru”

**Event description:**  Publishes Xethru breath sensor information and data.  

**Event data:**

1. **devicetype** - “Xethru”
2. **locationID** - UID corresponding to each bathroom and must batch with the row entry on the backend locations table. Current locationID entries look thus “REACH_1”, “REACH_3”, “EastsideWorks”.
3. **deviceID** - currently redundant and will be removed in future versions of the firmware. Set to "42" to avoid issues with null string arrays.
4. **distance** -
5. **RPM** - respirations per minute
6. **movement_slow** -
7. **movement_fast** -
8. **state_code** -

### “Xethru Error”

**Event description:**  Publishes error messages for/from the Xethru.  Most common error messages are “CRC Mismatch” and “Buffer Overflow”, but there are others in the v1.0 code.

With an ok from Sampath, Heidi has suppressed all Xethru error messages except those that appear during Argon bootup.  This will hopefully reduce the number of events published to the cloud and keep us under the publish rate limit set by Particle.

**Event data:**

N/A

### “Current Xethru Config Settings”

**Event description:**  Publishes what it says on the box!  If you have entered e for echo into the console function, it will read the current Xethru config settings being used and publish them to the cloud.

**Event data:**

1. **LED** 
    - 0: OFF
    - 1: SIMPLE
    - 2: FULL
2. **max_detect** - maximum range (meters) for detecting movement
3. **min_detect** - minimum range (meters) for detecting movement
4. **noisemap**
    - 0: FORCE INITIALIZE NOISEMAP ON RESET
    - 1: ADAPTIVE NOISEMAP ON
    - 2: USE DEFAULT NOISEMAP
5. **sensitivity** - scale goes from 0 to 10

### “Current SSIDs”

**Event description:**  Publishes what it says on the box!  If you have entered the correct password to the console function, this event will be published.  It will contain the 5 SSIDs currently stored in memory (including the Brave diagnostics network), and the SSID that the Particle is currently connected to.

**Event data:**

1. **mySSIDs[0]** - string containing this password
2. **mySSIDs[1]** - string containing this password
Etc up to mySSIDs[4]
3. **Connected to:** - string containing SSID the Particle WiFi module is currently connected to

### “Current wifi passwords”

**Event description:**  Publishes what it says on the box!  If you have entered the correct password to the console function, this event will be published.  It will contain the 5 wifi passwords currently stored in memory (including the Brave diagnostics network), and the SSID that the Particle is currently connected to.

Note: I can’t create an event that publishes the current password being used, since the Particle WiFi module does not provide that information.  Only WiFi.SSID() is offered for security reasons.

**Event data:**

1. **myPasswords[0]** - string containing this password
2. **myPasswords[1]** - string containing this password
Etc up to myPasswords[4]
3. **Connected to:** - string containing SSID the Particle WiFi module is currently connected to

### “Wifi Disconnect Warning”

**Event description:**  If the Particle loses wifi connection, the firmware will call the connectToWifi() function, and this will increment through the five different stored wifi credentials until connection to one of them is successful.

If reconnection is successful this event will be published, warning that there was a disconnection and providing the length of the disconnection in seconds.  If the Particle cannot reconnect obviously this event will not be published, but at this point you have bigger problems and this event can’t help you...

**Event data:**

1. Length of disconnect in seconds - integer number of seconds

### “Door”

**Event description:**  “Door” presently refers to the IM21 bluetooth low energy door sensor.  It sends out bluetooth advertising data whenever the door is open or closed.  Note that it does not advertise continuously, only when it detects an event (opened or closed).  Whenever the Particle receives advertising data from the door sensor, it publishes it to the cloud.

**Event data:**

1. **doorID** - three byte device ID for the IM21, broadcast by the device in its advertising data, and also contained on a sticker on the device.  See IM21 Bluetooth Door Sensor Settings section for more information re door’s device ID.
2. **Data** - a single byte indicating whether door is opened or closed. 0x00 = closed, 0x02 = open.
3. **Control** - a single byte that increments by 1 every time the door sensor advertises data. Useful to count events (for battery usage calculations) and to ensure Particle does not publish duplicate events.

### “Door Warning”

**Event description:**  This event is published if the Particle sees that a door event has been missed, or if it is seeing a new door ID transmitting data for the first time, or if the Particle has powered on/rebooted.  If you have not changed the door ID via the console function, and the Particle has not recently power cycled, you have missed door data!

The IM21 door sensor increments a control byte by 0x01 every time advertising data is transmitted. The Particle will publish a “Door Warning” event if the control byte for the “Door” event it is publishing to the cloud is 0x02 or more than the control byte for the previous “Door” event published.

**Event data:**

1. **doorID** - three byte device ID for the IM21, broadcast by the device in its advertising data, and also contained on a sticker on the device.  See [Door Sensor Settings](#door-sensor-settings) section for more information re door’s device ID.
2. **Data** - a single byte indicating whether door is opened or closed. 
    - 0x00 = closed
    - 0x02 = open.
3. **Control** - a single byte that increments by 1 every time the door sensor advertises data. Useful to count events (for battery usage calculations) and to ensure Particle does not publish duplicate events.
4. **Warning** - text string saying “Missed a door event, OR new doorID, OR Particle was rebooted.”

#### Current Door Sensor ID

**Event description:**  Publishes what it says on the box!  If you have entered e for echo into the console function, it will read the device ID of the door sensor the Particle is currently reading advertising data from, and publish it to the cloud.

**Event data:**

1. **Byte1** - first byte of door ID, in hex
2. **Byte2** - second byte of door ID, in hex
3. **Byte3** - third byte of door ID, in hex

#### "spark/device/diagnostics/update" (#spark)

**Event description:**  This event is triggered by the publishVitals() function. More documentation on this function can be found [here](https://docs.particle.io/reference/device-os/firmware/argon/#particle-publishvitals-).

**Event data:**

```json
{
  "device": {
    "network": {
      "signal": {
        "at": "Wi-Fi",
        "strength": 100,
        "strength_units": "%",
        "strengthv": -42,
        "strengthv_units": "dBm",
        "strengthv_type": "RSSI",
        "quality": 100,
        "quality_units": "%",
        "qualityv": 50,
        "qualityv_units": "dB",
        "qualityv_type": "SNR"
      },
      "connection": {
        "status": "connected",
        "error": 1024,
        "disconnects": 38,
        "attempts": 1,
        "disconnect_reason": "reset"
      }
    },
    "cloud": {
      "connection": {
        "status": "connected",
        "error": 0,
        "attempts": 1,
        "disconnects": 58,
        "disconnect_reason": "error"
      },
      "coap": {
        "transmit": 0,
        "retransmit": 0,
        "unack": 0,
        "round_trip": 0
      },
      "publish": {
        "rate_limited": 14
      }
    },
    "system": {
      "uptime": 3714727,
      "memory": {
        "used": 40200,
        "total": 82944
      }
    }
  },
  "service": {
    "device": {
      "status": "ok"
    },
    "cloud": {
      "uptime": 250329,
      "publish": {
        "sent": 250074
      }
    },
    "coap": {
      "round_trip": 2194
    }
  }
}
```
## Webhook Templates

To send sensor data and alerts to the node application on the brave sensor backend, we require a webhook to take the data from the particle publish message and send a HTTP/S post request to the backend server. This section contains the various templates that are used for sending various data payloads to the backed corresponding to various sensors 

### Xethru Template

The XeThru Template uses a custom webhook since the request format of the webhook is a `web form` and not a JSON object. This is technical debt from the initial implementation of sensor which hasn't been changed since the XeThru module support is inactive. 

#### Xethru publish message
``` c++
  snprintf(buf, sizeof(buf), "{\"devicetype\":\"%s\", \"location\":\"%s\", \"device\":\"%d\", \"distance\":\"%f\", \"rpm\":\"%f\", \"slow\":\"%f\", \"fast\":\"%f\", \"state\":\"%lu\"}", 
        deviceType, locationID, deviceID, message->distance, message->rpm, message->movement_slow, message->movement_fast, message->state_code);

  Particle.publish("XeThru", buf, PRIVATE); 
```

#### Xethru webhook 
```json
{
    "event": "XeThru",
    "responseTopic": "{{PARTICLE_DEVICE_ID}}/hook-response/{{PARTICLE_EVENT_NAME}}",
    "url": "https://sensors.brave.coop/api/xethru",
    "requestType": "POST",
    "noDefaults": true,
    "rejectUnauthorized": true,
    "form": {
        "distance": "{{{distance}}}",
        "rpm": "{{{rpm}}}",
        "devicetype": "{{{devicetype}}}",
        "locationid": "{{{location}}}",
        "deviceid": "{{{deviceid}}}",
        "state": "{{{state}}}",
        "mov_f": "{{{fast}}}",
        "mov_s": "{{{slow}}}"
    }
}
```

### IM21 Template

The request format of the IM21 webhook template is JSON. As a result, the webhook template is not unique, and the only link to the particle publish message is simply the name of the event. At the backend, the sensor data can be extracted from the body of the request.

#### IM21 publish message

```c++
sprintf(doorPublishBuffer, "{ \"deviceid\": \"%02X:%02X:%02X\", \"data\": \"%02X\", \"control\": \"%02X\" }", 
        globalDoorID.byte1, globalDoorID.byte2, globalDoorID.byte3, currentDoorData.doorStatus, currentDoorData.controlByte);
Particle.publish("IM21 Data", doorPublishBuffer, PRIVATE);
```

#### IM21 webhook

```json
{
    "event": "IM21 Data",
    "responseTopic": "{{PARTICLE_DEVICE_ID}}/hook-response/{{PARTICLE_EVENT_NAME}}",
    "url": "https://sensors.brave.coop/api/door",
    "requestType": "POST",
    "noDefaults": false,
    "rejectUnauthorized": true
}
```

## Firmware State Machine

Since this is intended to run on the Boron, it does not have the wifi code or wifi console functions found in previous versions of the firmware.  The state machine firmware will have no config.h file or need to flash setup firmware first.  

### Firmware State Machine Setup

The firmware only needs to be flashed to a device once.  It will initialize state machine constants (timer lengths, INS threshold) to sensible default values, which can later be tweaked and configured via console functions.  It will initialize the door sensor ID to 0xAA 0xAA 0xAA.  This can be updated via console function once the device is connected to LTE.

### Important Constants and Settings

### Console Functions

### Published Messages

 