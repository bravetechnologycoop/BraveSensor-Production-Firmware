#include "Particle.h"
#include "odetect_config.h"
#include "xm132.h"

//***************************XM132 functions**********************************

/*****************called from setup() or setup() sub-functions*******************/

void xm132Setup(){

  //Start serial communication
  SerialRadar.begin(115200);

  Log.info("Entered XM132 Setup ");

  unsigned char write_value[4];

  //Start XM132 module
  write_value[0] = 0x00;
  write_value[1] = 0x00;
  write_value[2] = 0x00;
  write_value[3] = 0x03;
  writeToXM132(0x03,write_value);

  //wait half a second to give service time to halt
  delay(500);

/*  //Clear errors and status
  write_value[0] = START_SERVICE;
  write_value[1] = 0x00;
  write_value[2] = 0x04;
  write_value[3] = 0x00;
  writeToXM132(MAIN_CONTROL_REGISTER,write_value);

  //set XM132 to presence mode (0x400)
  write_value[0] = 0x00;
  write_value[1] = 0x00;
  write_value[2] = 0x04;
  write_value[3] = 0x00;
  writeToXM132(MODE_SELECTION_REGISTER, write_value);

  //Activate module and start presence mode
  write_value[0] = STOP_SERVICE;
  write_value[1] = 0x00;
  write_value[2] = 0x04;
  write_value[3] = 0x00;
  writeToXM132(MAIN_CONTROL_REGISTER, write_value);
*/
}

void writeToXM132(unsigned char address, unsigned char value[4]){

  Log.info("Entered writeToXM132");

  int len = 10;
  unsigned char register_write_request[len];
  unsigned char register_write_response[128]; //max size of serial buffer = 128 bytes

  register_write_request[0] = 0xCC;
  register_write_request[1] = 0x05;
  register_write_request[2] = 0x00;
  register_write_request[3] = 0xF9;
  register_write_request[4] = address;
  //reverse order of value's indicides for little endian
  register_write_request[5] = value[3];
  register_write_request[6] = value[2];
  register_write_request[7] = value[1];
  register_write_request[8] = value[0];
  register_write_request[9] = 0xCD;

  Log.info("starting write");
  Log.info("register_write_request[%d] = %0X", register_write_request[9]);

  SerialRadar.write(register_write_request, len);

  //wait around for response
  //gets trapped in this loop
/*  while(!SerialRadar.available()){
    Log.info("heidi code is waiting for ack");
  }
*/
  // Wait 500 ms if nothing is available yet
  if (!SerialRadar.available())
    delay(500);
    
  // Wait for start character
  while (SerialRadar.available()) 
  {

    Log.info("SerialRadar is now available");

    int numBytes = 12;

    for(int i = 0; i <= numBytes; i++){
      register_write_response[i] = SerialRadar.read();
    }

    for(int i = 0; i <= numBytes; i++){
      Log.info("Register write response[%d] = %02X",i, register_write_response[i]);
    }

  }

}


/*  int recv_len = 0;  //Number of bytes received

  // Wait 500 ms if nothing is available yet
  if (!SerialRadar.available())
    delay(500);
    
  // Wait for start character
  while (SerialRadar.available()) 
  {

    unsigned char c = SerialRadar.read();  // Get one byte from radar

    // If we receive an ESCAPE character, the next byte is never the real start character
    if (c == XT_ESCAPE)
    {
      // Wait for next byte and skip it.
      while (!SerialRadar.available());
      SerialRadar.read();
    }
    else if (c == XT_START) 
    {
      // If it's the start character we fill the first character of the buffer and move on
      xethru_recv_buf[0] = c;
      recv_len = 1;
      break;
    }

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())  delay(10);

  } //endwhile

  // Wait 10 ms if nothing is available yet
  if (!SerialRadar.available())
    delay(10);
    
  // Start receiving the rest of the bytes
  while (SerialRadar.available()) 
  {
    // read a byte
    unsigned char c = SerialRadar.read(); // Get one byte from radar

    // is it an escape byte?
    if (c == XT_ESCAPE)
    {
      // If it's an escape character next character in buffer is data and not special character:
      while (!SerialRadar.available());
      c = SerialRadar.read();
    }
    // is it the stop byte?
    else if (c == XT_STOP) {
      // Fill response buffer, and increase counter
      xethru_recv_buf[recv_len++] = c;
      break;  //Exit this loop 
    }

    if (recv_len >= RX_BUF_LENGTH) {
      //errorPublish("Buffer Overflow");
      SerialDebug.println("BUFFER OVERFLOW!");
      return -1;
    }
    
    // Fill response buffer, and increase counter
    xethru_recv_buf[recv_len++] = c;

    

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())
      delay(10);
  }

  // Print received data
  #if 0
  SerialDebug.print("Received: ");
  for (int i = 0; i < recv_len; i++) {
    SerialDebug.print(xethru_recv_buf[i], HEX);
    SerialDebug.print(" ");
  }
  SerialDebug.println(" ");
  #endif
  
  // If nothing was received, return 0.
  if (recv_len==0) {
    errorPublish("No message received");
    return 0; //Many sleep messages at once cause no message sometimes.
  }
  // If stop character was not received, return with error.
  if (xethru_recv_buf[recv_len-1] != XT_STOP) {
    xethru_recv_buf[recv_len++] = XT_STOP; // If the message is missing the Stop Byte, just append it at the end.
    //char boof[1024];
	//snprintf(boof, sizeof(boof), "Length of received buffer: %d", recv_len);
    //Particle.publish("Length", boof, PRIVATE); //Checks how far into the message it gets
    //return -1; //Too many legacy messages has some messages missing stop byte.
  }
*/


/*   if(SerialRadar.available()) {

    int numBytes = SerialRadar.available();

    SerialDebug.printlnf("numBytes = %d",numBytes); //this was printing 0 bytes

    for(int i = 0; i <= numBytes; i++){
      register_write_response[i] = SerialRadar.read();
    }

    for(int i = 0; i <= numBytes; i++){
      SerialDebug.println("Register write response: ");
      SerialDebug.printlnf("response[%d] = %02X",i, register_write_response[i]);
    }

  //}
*/



/*void readFromXM132(){
  


  // Start receiving the rest of the bytes
  while (SerialRadar.available()) 
  {
    // read a byte
    unsigned char c = SerialRadar.read(); // Get one byte from radar

    // is it an escape byte?
    if (c == XT_ESCAPE)
    {
      // If it's an escape character next character in buffer is data and not special character:
      while (!SerialRadar.available());
      c = SerialRadar.read();
    }
    // is it the stop byte?
    else if (c == XT_STOP) {
      // Fill response buffer, and increase counter
      xethru_recv_buf[recv_len++] = c;
      break;  //Exit this loop 
    }

    if (recv_len >= RX_BUF_LENGTH) {
      //errorPublish("Buffer Overflow");
      SerialDebug.println("BUFFER OVERFLOW!");
      return -1;
    }
    
    // Fill response buffer, and increase counter
    xethru_recv_buf[recv_len++] = c;

    

    // Wait 10 ms if nothing is available yet
    if (!SerialRadar.available())
      delay(10);
  }


}
*/
