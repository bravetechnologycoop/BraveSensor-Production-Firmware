#include "Particle.h"
#include "BraveSensor_firmware_config.h"
#include "xm132.h"

//***************************XM132 functions**********************************

/*****************called from loop() or loop() sub-functions*******************/

void checkXM132(){

  unsigned int detected;
  unsigned int score;
  unsigned int distance;
  static unsigned long lastPublish = 0;

  //publish data every 1.5 seconds
  if((millis()-lastPublish) > 1500){

    //clear errors and status bits
    Log.warn("Clear errors and status bits");
    unsigned char clear_bits[4] = CLEAR_STATUS_BITS;
    while(writeToXM132(MAIN_CONTROL_REGISTER,clear_bits));

    Log.warn("Check status register for service creation");

    if(waitForStatusReady(DATA_READY,3000)){
      Log.warn("Data Ready");
    } else{
      Log.error("Data Not Ready");
    }

    //read detection register
    detected = readFromXM132(DETECTED_REGISTER);
    if(detected == 0xFFFFFFFF) detected = 1;
    //read score register
    score = readFromXM132(SCORE_REGISTER);
    //read distance register
    distance = readFromXM132(DISTANCE_REGISTER);

    Log.warn("Detection = %u", detected);
    Log.warn("Score = %u", score);
    Log.warn("Distance (mm) = %u", distance);

/*    //create JSON
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
*/
    lastPublish = millis();

  } 


}

/*****************called from setup() or setup() sub-functions*******************/

void xm132Setup(){

  //Start serial communication
  SerialRadar.begin(115200);

  Log.info("Entered XM132 Setup ");

  //Stop service 
  Log.warn("Stop Service");
  unsigned char stop_service[4] = STOP_SERVICE;
  while(writeToXM132(MAIN_CONTROL_REGISTER,stop_service));

  //wait half a second to give service time to halt
  delay(500);

  //clear errors and status bits
  Log.warn("Clear errors and status bits");
  unsigned char clear_bits[4] = CLEAR_STATUS_BITS;
  while(writeToXM132(MAIN_CONTROL_REGISTER,clear_bits));

  //set mode to presence
  Log.warn("Set presence mode");
  unsigned char presence[4] = PRESENCE_SERVICE;
  while(writeToXM132(MODE_SELECTION_REGISTER,presence));

  //set range_start register
  Log.warn("Set range start register mode");
  unsigned char startRegisterAddr = 0x20;
  unsigned char range_start[4] = {0x00, 0x00, 0x00, 0x0A};
  while(writeToXM132(startRegisterAddr,range_start));

  //set range_length register
  Log.warn("Set range start register mode");
  unsigned char rangeRegisterAddr = 0x21;
  unsigned char range_length[4] = {0x00, 0x00, 0x00, 0x50};
  while(writeToXM132(rangeRegisterAddr,range_length));

  //start service
  Log.warn("Start service");
  unsigned char start_service[4] = START_SERVICE;
  while(writeToXM132(MAIN_CONTROL_REGISTER,start_service));

  //wait for module to be activated and created
  Log.warn("Check status register for service creation");
  if(waitForStatusReady(MODULE_CREATED_AND_ACTIVATED,3000)){
    Log.warn("Module created and activated");
  } else{
    Log.error("Module failed to activate!");
  }

  //read out start of sweep in mm
  unsigned int start;
  start = readFromXM132(START_REGISTER);
  Log.warn("Start of sweep (mm) = %u", start);

  //read out length of sweep in mm
  unsigned int length;
  length = readFromXM132(LENGTH_REGISTER);
  Log.warn("Length of sweep (mm) = %u", length);  

  Log.warn("XM132 setup complete");

}


unsigned int waitForStatusReady(unsigned int desiredStatus, unsigned int timeout){

  unsigned long startTime = millis();

  unsigned int status;

  //read status register until service created/activated, or timeout
  while((millis() - startTime) < timeout){

    //check register until you get a proper read response
    status = readFromXM132(STATUS_REGISTER);
    Log.info("Actual status read = 0x%08X", status);

    //check the response, is it what we want?
    if((status & desiredStatus) == desiredStatus){
      Log.warn("Desired status 0x%08X received",desiredStatus);
      return status;
    } else {
      Log.warn("Waiting for desired status flag: 0x%08X",desiredStatus);
    }
    
  }//end timeout while

  if((millis() - startTime) > timeout){ 
    Log.error("Timeout of %d milliseconds, desired status 0x%08X not found", timeout, desiredStatus);
  }

  return 0;

}


//input - address of register
//returns - data
unsigned int readFromXM132(unsigned char address){

  unsigned int data = 0;
  int loopFlag = -1;
  //whole function gets dumped in a while because for some reason you need to call every
  //command on the acconeer twice?  add a timeout option later for more robust code 
  while(loopFlag == -1){

    unsigned char register_read_response[10]; //max size of serial buffer = 128 bytes
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
    } 

    //Empty anything else the 128 byte serial read buffer might have been sent
    while (SerialRadar.available())  SerialRadar.read();

    //if response is correct, parse data, else print error and set loopFlag = -1
    if((register_read_response[3]==0xF6) && (register_read_response[4]==address)){
      Log.info("Successfully read from register address = 0x%02X:",address);
      //got the correct data, so stop looping
      loopFlag = 1;

      //data contained in bytes 5 through 8 of read response array
      //place them in their own register_data array, while reversing order 
      //of the bytes from little endian to big endian
      unsigned char register_data[4];
      for (int j = 0; j < 4; j++){
        register_data[j] = register_read_response[8-j];
        Log.info("register_data[%d] = 0x%02X", j, register_data[j]);
      }

      //convert 4 bytes to an int
      data = bytesToInt(register_data);

    } else {
      Log.info("Failed to read from register address = 0x%02X",address);
      loopFlag = -1;
    }

    //leaving this down here so I can see what response whether it was good or caused an error
    for(int k = 0; k < 10; k++){
      Log.info("register_read_response[%d] = 0x%02X", k, register_read_response[k]);
    }

  } //endwhile

  return data;

}

unsigned int bytesToInt(unsigned char myBytes[4]){

  unsigned int myInt;
  myInt = (myBytes[0] << 24) + (myBytes[1] << 16) + (myBytes[2] << 8) + myBytes[3];
  //Log.info("myInt = 0x%08X", myInt);
  //Log.info("myInt = %u", myInt);

  return myInt;

}

int writeToXM132(unsigned char address, unsigned char register_command[4]){

  int returnFlag = 1;

  //loop until writing from the register is successful
  while(returnFlag == 1) {

    unsigned char register_write_response[10]; //max size of serial buffer = 128 bytes
    unsigned char register_write_request[10] = {0xCC, 0x05, 0x00, 0xF9};

    register_write_request[4] = address;
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
    }

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

  }//end while

  return returnFlag;

}


