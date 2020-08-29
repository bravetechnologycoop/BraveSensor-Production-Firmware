


//*************global settings*****************


//#define PHOTON  //enables code for photon device
#define USE_SERIAL  //when used, displays serial debugging messages

//*************Wifi Settings*******************

//writes original ssid/passwords to flash - see documentation for
//when and how to use this

//#define WRITE_ORIGINALS  

//set initial SSID/password pairs here
//always use the "" string format
//Do not leave any SSID or password undefined: empty
//elements in char array will throw an error

#define CLIENTSSID1 "ClientSSID1"
#define CLIENTSSID2 "Testbed"
#define CLIENTSSID3 "ClientSSID1"
#define CLIENTSSID4 "ClientSSID1"

#define CLIENTPWD1 "ClientPWD1"
#define CLIENTPWD2 "fireweed5"
#define CLIENTPWD3 "ClientPWD1"
#define CLIENTPWD4 "ClientPWD1"

//*************Bluetooth Door Sensor Settings***********

#define DOORID_BYTE1 0x96
#define DOORID_BITE2 0x59
#define DOORid_BYTE3 0x27

//************XeThru Settings**************

#define LOCATIONID "HeidiTest"
#define DEVICEID "H"
#define DEVICETYPE "XeThru"

 // XeThru configuration variables
 //these can be made defines so they don't have to be global variables
 //they are used (and passed, why, if they're global you shouldn't have to do 
 //that, weird...) in xethru_configuration()'s sub-functions and in particle console
 //function get_configuration_values()
 #define XETHRU_LED_SETTING 0
 #define XETHRU_NOISEMAP_SETTING 0
 #define XETHRU_SENSITIVITY_SETTING 5
 #define XETHRU_MIN_DETECT_SETTING 0.5
 #define XETHRU_MAX_DETECT_SETTING 4
