#include "Particle.h"
#include "odetect_config.h"
#include "xm132.h"

//***************************XM132 functions**********************************

/*****************called from setup() or setup() sub-functions*******************/

void xm132Setup(){

  //Start serial communication
  SerialRadar.begin(115200);

  Log.trace("Entered XM132 Setup ");

  //make sure module is stopped
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, STOP_SERVICE, 0xCD};
  //clear errors and status bits
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, CLEAR_STATUS_BITS, 0xCD};
  //set mode to presence
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MODE_SELECTION_REGISTER, PRESENCE_SERVICE, 0xCD};
  //activate and start
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, START_SERVICE, 0xCD};

  //Stop service 
  unsigned char register_command[4] = STOP_SERVICE;
  writeToXM132(MAIN_CONTROL_REGISTER,register_command);

  //wait half a second to give service time to halt
  delay(500);

  //clear errors and status bits
  unsigned char register_command[4] = CLEAR_STATUS_BITS;
  writeToXM132(MAIN_CONTROL_REGISTER,register_command);

  //set mode to presence
  unsigned char register_command[4] = PRESENCE_SERVICE;
  writeToXM132(MAIN_CONTROL_REGISTER,register_command);

  //start service
  unsigned char register_command[4] = START_SERVICE;
  writeToXM132(MAIN_CONTROL_REGISTER,register_command);


  checkStatusRegister(MODULE_CREATED_AND_ACTIVATED,3);


}


void checkStatusRegister(unsigned char wantedBits, int timeout){

  unsigned long startTime = Time.now();

  while(1){
    unsigned char status[] = readFromXM132(STATUS_REGISTER);


  }

}

void writeToXM132(unsigned char address, unsigned char register_command[4]){

  Log.trace("Entered writeToXM132");

  static unsigned char register_write_response[10]; //max size of serial buffer = 128 bytes
  unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9};
  register_write_request[4] = address;
  for(int i = 5; i < 9; i++){
    register_write_request[i] = register_command[i-5];
  }
  register_write_request[9] = 0xCD;

  SerialRadar.write(register_write_request, 10);
  //Serial1.write(0xAA);

  while(SerialRadar.available()) {
    //write response should contain 0xF5 followed by address of register read, as per Acconeer docs
    for(int i = 0; i < 10; i++){
      register_write_response[i] = SerialRadar.read();
      Log.trace("register_write_response[%d] = 0x%02X", i, register_write_response[i]);
    }
    
    if((register_write_response[3]==0xF5) && (register_write_response[4]==address)){
      Log.trace("Successfully wrote to register address = 0x%02X",address);
    } else {
      Log.error("Failed to write to XM132! Register address = 0x%02X",address);
    }

  }

}

//input - address of register, returns - 5 bytes of register response data
unsigned char * readFromXM132(unsigned char address){

  Log.trace("Entered readFromXM132");

  static unsigned char register_read_response[10]; //max size of serial buffer = 128 bytes
  unsigned char register_read_request[6] = {0xCC, 0x01, 0x00, 0xF8};
  register_read_request[4] = address;
  register_read_request[5] = 0xCD;

  SerialRadar.write(register_read_request, 6);
  //Serial1.write(0xAA);

  while(SerialRadar.available()) {
    //read response contains 0xF6, register address, and 5 bytes of data as per Acconeer docs
    for(int i = 0; i < 10; i++){
      register_read_response[i] = SerialRadar.read();
      Log.trace("register_write_response[%d] = 0x%02X", i, register_read_response[i]);
    }
    
    if((register_read_response[3]==0xF6) && (register_read_response[4]==address)){
      Log.trace("Successfully read from register address = 0x%02X",address);
    } else {
      Log.error("Failed to read from XM132! Register address = 0x%02X",address);
    }
  }
  
  unsigned char register_data[5];

  for (int j = 0; j < 6, j++;){
    register_data[j] = register_read_response[j+5];
  }

  return register_read_response;

}
