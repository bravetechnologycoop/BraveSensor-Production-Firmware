/*
 * Brave firmware state machine for single Boron
 * written by Heidi Fedorak, Apr 2021
 * 
 * Edits for Xethru local state machine by James Seto
 *    -Check for SleepMessage data from XeThru module
 *    -Slight modifications to no new data check for heartbeat
*/

#include "Particle.h"
#include "stateMachine.h"
#include "im21door.h"
#include "xethru.h"
#include "flashAddresses.h"

//define and initialize state machine pointer
StateHandler stateHandler = state0_idle;

//define global variables so they are allocated in memory
unsigned long state1_timer;
unsigned long state2_duration_timer;
unsigned long state3_stillness_timer;
//initialize constants to sensible default values
unsigned long slow_threshold = SLOW_THRESHOLD;
unsigned long state1_max_time = STATE1_MAX_TIME;
unsigned long state2_max_duration = STATE2_MAX_DURATION;
unsigned long state3_max_stillness_time = STATE3_MAX_STILLNESS_TIME;
//except this one, we don't want to take the chance that random memory
//contents will initialize this to "on"
bool stateMachineDebugFlag = false;

void setupStateMachine(){

  //set up debug pins
  // pinMode(D2,OUTPUT);
  // pinMode(D3,OUTPUT);
  // pinMode(D4,OUTPUT);
  // pinMode(D5,OUTPUT);

  //default to not publishing debug logs
  stateMachineDebugFlag = 0;

}

void initializeStateMachineConsts(){

  uint16_t initializeConstsFlag;

  //Argon flash memory is initialized to all F's (1's)
  EEPROM.get(ADDR_INITIALIZE_SM_CONSTS_FLAG, initializeConstsFlag);
  Log.info("state machine constants flag is 0x%04X", initializeConstsFlag);

  if(initializeConstsFlag != INITIALIZE_STATE_MACHINE_CONSTS_FLAG){
    EEPROM.put(ADDR_SLOW_THRESHOLD, slow_threshold);
    EEPROM.put(ADDR_STATE1_MAX_TIME, state1_max_time);
    EEPROM.put(ADDR_STATE2_MAX_DURATION, state2_max_duration);
    EEPROM.put(ADDR_STATE3_MAX_STILLNES_TIME, state3_max_stillness_time);
    initializeConstsFlag = INITIALIZE_STATE_MACHINE_CONSTS_FLAG;
    EEPROM.put(ADDR_INITIALIZE_SM_CONSTS_FLAG, initializeConstsFlag);
    Log.info("State machine constants were written to flash on bootup.");
  }
  else{
    EEPROM.get(ADDR_SLOW_THRESHOLD, slow_threshold);
    EEPROM.get(ADDR_STATE1_MAX_TIME, state1_max_time);
    EEPROM.get(ADDR_STATE2_MAX_DURATION, state2_max_duration);
    EEPROM.get(ADDR_STATE3_MAX_STILLNES_TIME, state3_max_stillness_time);
    Log.info("State machine constants were read from flash on bootup.");
  }


}




void state0_idle(){

  //scan inputs
  doorData checkDoor;
  SleepMessage checkXT;
  //this returns the previous door event value until a new door event is received
  //on code boot up it initializes to returning 0x99
  checkDoor = checkIM21();
  //this returns 0.0 if the XeThru has no new data to transmit
  checkXT = checkXeThru();

  //do stuff in the state
  // digitalWrite(D2,LOW);
  // digitalWrite(D3,LOW);
  // digitalWrite(D4,LOW);
  // digitalWrite(D5,LOW);


  Log.info("You are in state 0, idle: Door status, movement_fast, movement_slow = 0x%02X, %f, %f",
          checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
  //default timer to 0 when state doesn't have a timer
  publishDebugMessage(0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, 0); 

  //fix outputs and state exit conditions accordingly
  if(((unsigned long)checkXT.movement_fast > FAST_THRESHOLD)
          && isDoorClosed(checkDoor.doorStatus)){

    Log.warn("In state 0, door closed and seeing movement, heading to state 1");
    publishStateTransition(0, 1, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    //zero the state 1 timer
    state1_timer = millis();
    //head to state 1
    stateHandler = state1_15sCountdown;

  }
  else {
    //if we don't meet the exit conditions above, we remain here
    //stateHandler = state0_idle;
  }

}

void state1_15sCountdown(){

  //scan inputs
  doorData checkDoor;
  SleepMessage checkXT;
  //this returns the previous door event value until a new door event is received
  //on code boot up it initializes to returning 0x99
  checkDoor = checkIM21();
  //this returns 0.0 if the XeThru has no new data to transmit
  checkXT = checkXeThru();

  //do stuff in the state
  // digitalWrite(D2,HIGH);
  Log.info("You are in state 1, 15s countdown: Door status, movement_fast, movement_slow, timer = 0x%02X, %f, %f, %ld",
          checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis() - state1_timer));
  publishDebugMessage(1, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis()-state1_timer));  


  //fix outputs and state exit conditions accordingly
  if((unsigned long)checkXT.movement_fast > 0 && ((unsigned long)checkXT.movement_fast < FAST_THRESHOLD)){

    Log.warn("no movement, you're going back to state 0 from state 1");
    publishStateTransition(1, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    stateHandler = state0_idle;

  }
  else if(isDoorOpen(checkDoor.doorStatus)){

    Log.warn("door was opened, you're going back to state 0 from state 1");
    publishStateTransition(1, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    stateHandler = state0_idle;

  }
  else if( millis() - state1_timer >= state1_max_time){

    Log.warn("door closed && motion for > Xs, going to state 2 from state1");
    publishStateTransition(1, 2, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    //zero the duration timer
    state2_duration_timer = millis();
    //head to duration state
    stateHandler = state2_duration;

  }
  else{

    //if we don't meet the exit conditions above, we remain here
    //stateHandler = state1_15sCountdown;
  
  }

}


void state2_duration(){

  //scan inputs
  doorData checkDoor;
  SleepMessage checkXT;
  //this returns the previous door event value until a new door event is received
  //on code boot up it initializes to returning 0x99
  checkDoor = checkIM21();
  //this returns 0.0 if the XeThru has no new data to transmit
  checkXT = checkXeThru();

  //do stuff in the state
  // digitalWrite(D3,HIGH);
  Log.info("You are in state 2, duration: Door status, movement_fast, movement_slow, timer = 0x%02X, %f, %f, %ld",
          checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis() - state2_duration_timer)); 
  publishDebugMessage(2, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis()-state2_duration_timer));  

  //fix outputs and state exit conditions accordingly
  if((unsigned long)checkXT.movement_fast > 0 && ((unsigned long)checkXT.movement_slow < slow_threshold)){

    Log.warn("Seeing stillness, going to state3_stillness from state2_duration");
    publishStateTransition(2, 3, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    //zero the stillness timer
    state3_stillness_timer = millis();
    //go to stillness state
    stateHandler = state3_stillness;

  }
  else if(isDoorOpen(checkDoor.doorStatus)){

    Log.warn("Door opened, session over, going to idle from state2_duration");
    publishStateTransition(2, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    stateHandler = state0_idle;

  }
  else if(millis() - state2_duration_timer >= state2_max_duration){

    Log.warn("See duration alert, going from state2_duration to idle after alert publish");
    publishStateTransition(2, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    Log.error("Duration Alert!!");
    Particle.publish("Duration Alert", "duration alert", PRIVATE);
    stateHandler = state0_idle;
  }
  else {
    //if we don't meet the exit conditions above hang out here
    //stateHandler = state2_duration;

  }


} //end state2_duration

void state3_stillness(){

  //scan inputs
  doorData checkDoor;
  SleepMessage checkXT;
  //this returns the previous door event value until a new door event is received
  //on code boot up it initializes to returning 0x99
  checkDoor = checkIM21();
  //this returns 0.0 if the XeThru has no new data to transmit
  checkXT = checkXeThru();

  //do stuff in the state
  // digitalWrite(D4,HIGH);
  Log.info("You are in state 3, stillness: Door status, movement_fast, movement_slow, timer = 0x%02X, %f, %f, %ld",
          checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis() - state3_stillness_timer));
  publishDebugMessage(3, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow, (millis()-state3_stillness_timer));   

  //fix outputs and state exit conditions accordingly
  if((unsigned long)checkXT.movement_slow > slow_threshold){

    Log.warn("motion spotted again, going from state3_stillness to state2_duration");
    publishStateTransition(3, 2, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    //zero the duration timer
    state2_duration_timer = millis();
    //go back to state 2, duration
    stateHandler = state2_duration;

  }
  else if(isDoorOpen(checkDoor.doorStatus)){

    Log.warn("door opened, session over, going from state3_stillness to idle");
    publishStateTransition(3, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    stateHandler = state0_idle;

  }
  else if(millis() - state3_stillness_timer >= state3_max_stillness_time){

    Log.warn("stillness alert, going from state3 to idle after publish");
    publishStateTransition(3, 0, checkDoor.doorStatus, checkXT.movement_fast, checkXT.movement_slow);
    Log.error("Stillness Alert!!");
    Particle.publish("Stillness Alert", "stillness alert!!!", PRIVATE);
    stateHandler = state0_idle;
  }
  else {
    //if we don't meet the exit conditions above, we remain here
    //stateHandler = state3_stillness;

  }

} //end state3_stillness

void publishStateTransition(int prevState, int nextState, unsigned char doorStatus, float fastValue, float slowValue){

    if(stateMachineDebugFlag){

      //from particle docs, max length of publish is 622 chars, I am assuming this includes null char
      char stateTransition[622];
      snprintf(stateTransition, sizeof(stateTransition), 
              "{\"prev_state\":\"%d\", \"next_state\":\"%d\", \"door_status\":\"0x%02X\", \"xethru_fast_val\":\"%f\", \"xethru_slow_val\":\"%f\"}", prevState, nextState, doorStatus, fastValue, slowValue); 
      Particle.publish("State Transition", stateTransition, PRIVATE);

    }

}

void publishDebugMessage(int state, unsigned char doorStatus, float fastValue, float slowValue, unsigned long timer){

    static unsigned long lastDebugPublish = 0;

    if(stateMachineDebugFlag && (millis()-lastDebugPublish) > DEBUG_PUBLISH_INTERVAL){

      //from particle docs, max length of publish is 622 chars, I am assuming this includes null char
      char debugMessage[622];
      snprintf(debugMessage, sizeof(debugMessage), 
              "{\"state\":\"%d\", \"door_status\":\"0x%02X\", \"xethru_fast_val\":\"%f\", \"xethru_slow_val\":\"%f\", \"timer_status\":\"%ld\"}", state, doorStatus, fastValue, slowValue, timer); 
      Particle.publish("Debug Message", debugMessage, PRIVATE);
      lastDebugPublish = millis();

    }

}

void getHeartbeat(){

    static unsigned long lastHeartbeatPublish = 0;
    unsigned long xeThruTimeDiff = 0;
    unsigned long doorTimeDiff = 0;
    static unsigned long currXeThruTimestamp = 0;
    static unsigned long prevXeThruTimestamp = 0;
    static unsigned long currDoorTimestamp = 0;
    static unsigned long prevDoorTimestamp = 0;
    float xeThruStatus = 0;
    unsigned char doorStatus;

    static SleepMessage currXeThruHeartbeat = {0,0,0,0};
    static doorData currDoorHeartbeat = {0x99,0x99,0};

    //call over and over again to get the most recent value in the heartbeat interval
    //make static so most recent value is stored.  Ditto door data.
    currXeThruHeartbeat = checkXeThru();
    currDoorHeartbeat = checkIM21();
    if((millis()-lastHeartbeatPublish) > SM_HEARTBEAT_INTERVAL){

      doorStatus = currDoorHeartbeat.doorStatus;
      xeThruStatus = currXeThruHeartbeat.movement_fast;

      currXeThruTimestamp = currXeThruHeartbeat.timestamp;
      currDoorTimestamp = currDoorHeartbeat.timestamp;

      if(currXeThruTimestamp == prevXeThruTimestamp){
        //if time stamps are the same, no new data
        xeThruTimeDiff = 0;
        xeThruStatus = 0;
        prevXeThruTimestamp = currXeThruTimestamp;
      }
      else{
        //timestamps different and no 0 data, so report new data point
        xeThruTimeDiff = millis() - currXeThruTimestamp;
        prevXeThruTimestamp = currXeThruTimestamp;
      }


      if(doorStatus == 0x99){
        //when door status is 0x99, there have been no door events since bootup
        doorTimeDiff = 0;
        prevDoorTimestamp = currDoorTimestamp;
      }
      else if(currDoorTimestamp == prevDoorTimestamp){
        //timestamps the same so no new data
        doorTimeDiff = 0;
        doorStatus = 0x99;
        prevDoorTimestamp = currDoorTimestamp;
      }
      else {
        //timestamps different, and door not 0x99, must be new data
        doorTimeDiff = millis() - currDoorTimestamp;
        prevDoorTimestamp = currDoorTimestamp;
      }
      
      //from particle docs, max length of publish is 622 chars, I am assuming this includes null char
      char heartbeatMessage[622];
      snprintf(heartbeatMessage, sizeof(heartbeatMessage),
                " {\"door_status\":\"0x%02X\", \"door_time\":\"%ld\", \"XeThru_status\":\"%f\", \"XeThru_time\":\"%ld\" }", doorStatus, doorTimeDiff, xeThruStatus, xeThruTimeDiff); 
      Particle.publish("Heartbeat", heartbeatMessage, PRIVATE);
      Log.warn(heartbeatMessage);
      lastHeartbeatPublish = millis();

    }

    

}
