#include "Particle.h"
#include "odetect_config.h"
#include "xm132.h"

//***************************XM132 functions**********************************

/*****************called from loop() or loop() sub-functions*******************/

void checkXM132(){

  static int detected;
  static int score;
  static int distance;
  static unsigned long lastPublish = 0;

  //read detection register
  do {
    detected = readFromXM132(DETECTED_REGISTER);
    Log.info("Actual detection read = 0x%02X", detected);
    } while(detected != -1);

  //read score register
  do {
    score = readFromXM132(SCORE_REGISTER);
    Log.info("Actual score read = 0x%02X", score);
    } while(score != -1);

  //read distance register
  do {
    distance = readFromXM132(DISTANCE_REGISTER);
    Log.info("Actual distance read = 0x%02X", distance);
    } while(distance != -1);

  //publish data every 1.5 seconds
  if((millis()-lastPublish) > 1500){

    //create JSON
    char data[1024];
    memset(data, 0, sizeof(data));
    JSONBufferWriter writer(data, sizeof(data) - 1);
    writer.beginObject();
      writer.name("detected").value(detected);
      writer.name("score").value(score);
      writer.name("distance (mm)").value(distance);    
      writer.endArray();
    writer.endObject();

    //publish to cloud
    Particle.publish("XM132", data, PRIVATE);
    lastPublish = millis();
  } 


}

/*****************called from setup() or setup() sub-functions*******************/

void xm132Setup(){

  //Start serial communication
  SerialRadar.begin(115200);

  Log.info("Entered XM132 Setup ");

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
  Log.info("Check status register for service creation");
  waitForStatusReady(MODULE_CREATED_AND_ACTIVATED,3000);

}


void waitForStatusReady(int desiredStatus, unsigned int timeout){

  unsigned long startTime = millis();

  int status = -1;

  //read status register until service created/activated, or timeout
  while((millis() - startTime) < timeout){

    //check register until you get a proper read response
    do {
      status = readFromXM132(STATUS_REGISTER);
      Log.info("Actual status read = 0x%02X", status);
      } while(status != -1);

    //check the response, is it what we want?
    if((status & desiredStatus) == desiredStatus){
      Log.info("Desired status 0x%02X received",status);
      break;
    }
    
  }//end timeout while

  Log.error("Timeout of %d seconds, status 0x%02X not received", timeout, status);

}


//input - address of register
//returns - data
int readFromXM132(unsigned char address){

  int data = -1;
  //whole function gets dumped in a do-while because for some reason you need to call every
  //command on the acconeer twice?  add a timeout option later for more robust code 
  do{

    static unsigned char register_read_response[10]; //max size of serial buffer = 128 bytes
    unsigned char register_read_request[6] = {0xCC, 0x01, 0x00, 0xF8};
    register_read_request[4] = address;
    register_read_request[5] = 0xCD;

    SerialRadar.write(register_read_request, 6);
    //Serial1.write(0xAA);

    for(int l = 0; l < 6; l++){
      Log.info("register_read_request[%d] = 0x%02X", l, register_read_request[l]);
    }

    while(SerialRadar.available()) {
      //read response contains 0xF6, register address, and 5 bytes of data as per Acconeer docs
      for(int i = 0; i < 10; i++){
        register_read_response[i] = SerialRadar.read();
      }
    } //end serial reading while

    //Empty anything else the 128 byte serial read buffer might have been sent
    while (SerialRadar.available())  SerialRadar.read();

    //if response is correct, parse data, else print error and set data = -1
    if((register_read_response[3]==0xF6) && (register_read_response[4]==address)){
      Log.info("Successfully read from register address = 0x%02X:",address);

      //data contained in bytes 5 through 9 of read response array
      //place them in their own register_data array, while reversing order 
      //of the bytes from little endian to big endian
      Log.info("Register data is:");
      static unsigned char register_data[4];
      for (int j = 0; j < 4; j++){
        register_data[j] = register_read_response[8-j];
        Log.info("register_data[%d] = 0x%02X", j, register_data[j]);
      }

      //convert 4 bytes to an int
      data = bytesToInt(register_data);

    } else {
      Log.error("Failed to read from register address = 0x%02X",address);
      data = -1;
    }

    //leaving this down here so I can see what response whether it was good or caused an error
    for(int k = 0; k < 10; k++){
      Log.info("register_read_response[%d] = 0x%02X", k, register_read_response[k]);
    }

  }while(data != -1);
    
  return data;

}

int bytesToInt(unsigned char myBytes[4]){

  int myInt = 0;
  myInt = (myBytes[0] << 24) + (myBytes[1] << 16) + (myBytes[2] << 8) + myBytes[3];
  return myInt;

}

int writeToXM132(unsigned char address, unsigned char register_command[4]){

  int returnFlag = 1;

  //loop until writing from the register is successful
  while(returnFlag == 1) {

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
      Log.error("Failed to write to register address = 0x%02X",address);
      returnFlag = 1;
    }

    for(int l = 0; l < 10; l++){
      Log.info("register_write_response[%d] = 0x%02X", l, register_write_response[l]);
    }  

  }//end while

  return returnFlag;

}


