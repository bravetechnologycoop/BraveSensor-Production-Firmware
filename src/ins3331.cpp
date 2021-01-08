/*
 * Project Particle_Innosent
 * Description:
 * Author: Sampath Satti  
 * Date: 29 May 2020
 */

#include "Particle.h"
#include "BraveSensor_firmware_config.h"
#include "ins3331.h"

// loop() function and subfunctions
void checkINS3331() {

  static unsigned char ins3331_recv_buf[20];  
  static int recv_buf_index = 0;
  static unsigned long int last_publish;

  static int inphase;
  static int quadrature;
  static String iValues;
  static String qValues;

  // read from port 1, send to port 0:
  if (SerialRadar.available())
  {
    //get a character
    uint8_t inByte = SerialRadar.read();
    //check if we're at the start of a frame, if so set buffer index to 0
    if (inByte == 0xA2){
      recv_buf_index = 0;
    }

    //put character in receive buffer
    ins3331_recv_buf[recv_buf_index]=inByte;

    //if buffer index = 13, frame has completed transmission so we can now parse
    //the transmitted data
    if(recv_buf_index == 13){ 

      //extract phase value from 7th and 8th bytes of the slave frame
      inphase = ((int(ins3331_recv_buf[7]) << 8) & 0xff00) + (int(ins3331_recv_buf[8]));
      inphase=twos_comp(inphase, 16);
      //load phase value into a String for publishing
      if(iValues.length()<200){
        iValues.concat(inphase);
        iValues.concat(',');
      }
      //extract quadrature value from 9th and 10th bytes of the slave frame
      quadrature = ((int(ins3331_recv_buf[9]) << 8) & 0xff00) + (int(ins3331_recv_buf[10]));
      quadrature=twos_comp(quadrature, 16);
      //load quadrature value into a String for publishing
      if(qValues.length()<200){
        qValues.concat(quadrature);
        qValues.concat(',');
      }
    }

    recv_buf_index++;
  }

  //publish the strings of phase & quadrature values every 1.5 seconds
  if((millis()-last_publish) > 1500){
    //publish to cloud
    cloudPublishINSdata(iValues, qValues);

    //print to USB serial
    usbSerialPrintINSdata(iValues, qValues);

    last_publish = millis();
  }

}

void usbSerialPrintINSdata(String iValues, String qValues){

  //create string for printing
  String data = "{ \"deviceid\": ";
  data.concat(INS_DEVICEID);
  data.concat("\"inPhase\": ");
  data.concat(iValues);
  data.concat(", ");
  data.concat("\"quadrature\": ");
  data.concat(qValues);
  data.concat(", ");
  data.concat("}");

  //print to usb serial
  SerialUSB.println(data);

  //log for debugging
  Log.info("INS data actually published:");
  Log.info(iValues, qValues);

}


void cloudPublishINSdata(String iValues, String qValues){

  //create string for publishing
  String data = "{ \"deviceid\": ";
  data.concat(INS_DEVICEID);
  data.concat("\"inPhase\": ");
  data.concat(iValues);
  data.concat(", ");
  data.concat("\"quadrature\": ");
  data.concat(qValues);
  data.concat(", ");
  data.concat("}");

  //publish to cloud
  Particle.publish("Radar", data, PRIVATE);

  //log for debugging
  Log.info("INS data actually published:");
  Log.info(iValues, qValues);

}

int twos_comp(int val, int bits){
  if((val & (1 << (bits - 1))) != 0){
    val = val - (1 << bits); 
  }
  return val;
}

//setup function & subfunctions
void ins3331Setup(){

  SerialRadar.begin(38400, SERIAL_8N1); 
  radar_stop();
  radar_start();

}

//
void radar_stop(){
  
  //Buffer for sending data to radar.
  //sending only done in radar_start() and radar_stop(), so 
  //this can be declared locally 
  unsigned char ins3331_send_buf[20];

  ins3331_send_buf[0] = 0x11; 
  ins3331_send_buf[1] = 0xA2; 
  ins3331_send_buf[2] = RADAR_ADDRESS; 
  ins3331_send_buf[3] = 0x01; 
  ins3331_send_buf[4] = 0xE4;
  ins3331_send_buf[5] = 0x00; 
  ins3331_send_buf[6] = 0x00; 
  ins3331_send_buf[7] = 0x00; 
  ins3331_send_buf[8] = 0x00;
  ins3331_send_buf[9] = 0x00; 
  ins3331_send_buf[10] = 0x00; 
  ins3331_send_buf[11] = 0x00; 
  ins3331_send_buf[12] = 0x00; 
  ins3331_send_buf[13] = 0x65; 
  ins3331_send_buf[14] = 0x16;
  SerialRadar.write(ins3331_send_buf, 11);
}

void radar_start(){

  //Buffer for sending data to radar.
  //sending only done in radar_start() and radar_stop(), so 
  //this can be declared locally 
  unsigned char ins3331_send_buf[20];   

  ins3331_send_buf[0] = 0x11; 
  ins3331_send_buf[1] = 0xA2; 
  ins3331_send_buf[2] = 0x80; 
  ins3331_send_buf[3] = 0x01; 
  ins3331_send_buf[4] = 0xEB;
  ins3331_send_buf[5] = 0x00; 
  ins3331_send_buf[6] = 0x00; 
  ins3331_send_buf[7] = 0x00; 
  ins3331_send_buf[8] = 0x00;
  ins3331_send_buf[9] = 0x00; 
  ins3331_send_buf[10] = 0x00; 
  ins3331_send_buf[11] = 0x00; 
  ins3331_send_buf[12] = 0x00; 
  ins3331_send_buf[13] = 0x6C; 
  ins3331_send_buf[14] = 0x16;
  SerialRadar.write(ins3331_send_buf, 11);
}
