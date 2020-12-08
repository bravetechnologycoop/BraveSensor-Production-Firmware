#include "Particle.h"
#include "odetect_config.h"
#include "xm132.h"

//***************************XM132 functions**********************************

/*****************called from setup() or setup() sub-functions*******************/

void xm132Setup(){

  //Start serial communication
  SerialRadar.begin(115200);

  Log.info("Entered XM132 Setup ");

  //make sure module is stopped
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, STOP_SERVICE, 0xCD};
  //clear errors and status bits
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, CLEAR_STATUS_BITS, 0xCD};
  //set mode to presence
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MODE_SELECTION_REGISTER, PRESENCE_SERVICE, 0xCD};
  //activate and start
  //static unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9, MAIN_CONTROL_REGISTER, START_SERVICE, 0xCD};

  //Stop service 
  Log.info("Stop Service(register, value): 0x03, 0x00");
  unsigned char stop_service[4] = STOP_SERVICE;
  while(writeToXM132(MAIN_CONTROL_REGISTER,stop_service));

  //wait half a second to give service time to halt
  delay(500);

  //clear errors and status bits
  Log.info("Clear errors and status bits(register, value): 0x03, 0x04");
  unsigned char clear_bits[4] = CLEAR_STATUS_BITS;
  while(writeToXM132(MAIN_CONTROL_REGISTER,clear_bits));

  //set mode to presence
  Log.info("Set distance mode(register, value): 0x02, 0x200");
  unsigned char presence[4] = DISTANCE_SERVICE;
  while(writeToXM132(MODE_SELECTION_REGISTER,presence));

  //start service
  Log.info("Start service(register, value): 0x03, 0x03");
  unsigned char start_service[4] = START_SERVICE;
  while(writeToXM132(MAIN_CONTROL_REGISTER,start_service));

  //unsigned char* status;
  while(readFromXM132(STATUS_REGISTER));

  //checkStatusRegister(MODULE_CREATED_AND_ACTIVATED,3);


}


/*void checkStatusRegister(unsigned char wantedBits, int timeout){

  unsigned long startTime = Time.now();
  unsigned char * status;

  while((Time.now() - startTime) < 3000){

    status = readFromXM132(STATUS_REGISTER);
    
  }

}
*/

//input - address of register, returns - 5 bytes of register response data
int readFromXM132(unsigned char address){

  static unsigned char register_read_response[10]; //max size of serial buffer = 128 bytes
  unsigned char register_read_request[6] = {0xCC, 0x01, 0x00, 0xF8};
  int returnFlag = 1;
  register_read_request[4] = address;
  register_read_request[5] = 0xCD;

  SerialRadar.write(register_read_request, 6);
  //Serial1.write(0xAA);

  while(SerialRadar.available()) {
    //read response contains 0xF6, register address, and 5 bytes of data as per Acconeer docs
    for(int i = 0; i < 10; i++){
      register_read_response[i] = SerialRadar.read();
    }
  } //end serial reading while

  //Empty anything else the 128 byte serial read buffer might have been sent
  while (SerialRadar.available())
    SerialRadar.read();

  for(int k = 0; k < 10; k++){
    Log.info("register_read_response[%d] = 0x%02X", k, register_read_response[k]);
  }
  
  //data contained in bytes 5 through 9 of read response array
  //place them in their own register_data array, while reversing order 
  //of the bytes from little endian to big endian
  Log.info("Register data is:");
  static unsigned char register_data[4];
  for (int j = 0; j < 4; j++){
    register_data[j] = register_read_response[8-j];
    Log.info("register_data[%d] = 0x%02X", j, register_data[j]);
  }

  if((register_read_response[3]==0xF6) && (register_read_response[4]==address)){
    Log.info("Successfully read from register address = 0x%02X",address);
    returnFlag = 0;
  } else {
    Log.info("Failed to read from register address = 0x%02X",address);
    returnFlag = 1;
  }

  return returnFlag;

}

int writeToXM132(unsigned char address, unsigned char register_command[4]){

  int returnFlag = 1;
  static unsigned char register_write_response[10]; //max size of serial buffer = 128 bytes
  unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9};

  register_write_request[4] = address;
/*  //load command into register_write_request[5:8] in reverse order to get 
  //little endian format...
  for(int i = 0; i <= 4; i++){
    register_write_request[8-i] = register_command[i];
  }
*/
  //using the loop commented out above causes register_write_request[4] to 
  //be overwritten for some weird reason, it always ends up as 0xC4 below,
  //despite being shown set equal to 0x02 above the for loop. Setting them
  //equal directly works:
  register_write_request[5] = register_command[3];
  register_write_request[6] = register_command[2];
  register_write_request[7] = register_command[1];
  register_write_request[8] = register_command[0];
  register_write_request[9] = 0xCD;

  for(int j = 0; j < 10; j++){
    Log.info("register_write_request[%d] = 0x%02X", j, register_write_request[j]);
  }

  SerialRadar.write(register_write_request, 10);
  //Serial1.write(0xAA);

  //load the response
  while(SerialRadar.available()) {
    for(int k = 0; k < 10; k++){
      register_write_response[k] = SerialRadar.read();     
    }
  }// end serial-reading while

  //Empty anything else the 128 byte serial read buffer might have been sent
  while (SerialRadar.available())
    SerialRadar.read();

  //check the response we want to keep for the correct bits: write response[3] should 
  //contain 0xF5 followed by address of register read, as per Acconeer docs and example:
  if((register_write_response[3]==0xF5) && (register_write_response[4]==address)){
    Log.info("Successfully wrote to register address = 0x%02X",address);
    returnFlag = 0;
  } else {
    Log.info("Failed to write to register address = 0x%02X",address);
    returnFlag = 1;
  }

  for(int l = 0; l < 10; l++){
    Log.info("register_write_response[%d] = 0x%02X", l, register_write_response[l]);
  }  

  return returnFlag;

}


