/*
 * Project Particle_Innosent
 * Description:
 * Author: Sampath Satti  
 * Date: 29 May 2020
 */

#include "Particle.h"
#include "odetect_config.h"
#include "ins3331.h"

// loop() function and subfunctions
void checkINS3331() {

  static unsigned char ins3331_recv_buf[20];  
  static int frame_count = 0;
  static unsigned long int last_publish;

  static int inphase;
  static int quadrature;
  static String iValues;
  static String qValues;

  // read from port 1, send to port 0:
  if (SerialRadar.available())
  {
    uint8_t inByte = SerialRadar.read();
    if (inByte == 0xA2){
      frame_count = 0;
    }
    ins3331_recv_buf[frame_count]=inByte;

    if(frame_count == 13){ 
      //ins3331_recv_buf[7]+ins3331_recv_buf[8]
      inphase = ((int(ins3331_recv_buf[7]) << 8) & 0xff00) + (int(ins3331_recv_buf[8]));
      inphase=twos_comp(inphase, 16);
      if(iValues.length()<200){
        iValues.concat(inphase);
        iValues.concat(',');
      }
      quadrature = ((int(ins3331_recv_buf[9]) << 8) & 0xff00) + (int(ins3331_recv_buf[10]));
      quadrature=twos_comp(quadrature, 16);
      if(qValues.length()<200){
        qValues.concat(quadrature);
        qValues.concat(',');
      }
    }
    frame_count++;
  }
  if((millis()-last_publish) > 1500){
    String data = publishINSdata(iValues, qValues);
    Particle.publish("Radar", data, PRIVATE);
    #if defined(SERIAL_DEBUG)
    SerialDebug.println("INS data actually published:");
    SerialDebug.printlnf(iValues, qValues);
    SerialDebug.printlnf("%02x:%02x:%04x", ins3331_recv_buf[7], ins3331_recv_buf[8], ((int(ins3331_recv_buf[7]) << 8) & 0xff00) + (int(ins3331_recv_buf[8])) );
    SerialDebug.printlnf("inphase %d, quadrature %d", inphase, quadrature);    
    #endif
    last_publish = millis();
    iValues = ' ';
    qValues = ' ';
  }
}

String publishINSdata(String iValues, String qValues){
  String data = "{ \"deviceid\": ";
  data.concat(INS_DEVICEID);
  data.concat("\"inPhase\": ");
  data.concat(iValues);
  data.concat(", ");
  data.concat("\"quadrature\": ");
  data.concat(qValues);
  data.concat(", ");
  data.concat("}");
  return data;
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