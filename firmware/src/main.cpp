#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <CmdMessenger.h>
#include <Adafruit_PWMServoDriver.h>

#define NUM_TELESCOPES   3
#define NUM_SERVOS       8
#define MOVE_WAIT_TIME 800

#define SWITCH_TELE0     2 
#define SWITCH_TELE1     3   
#define SWITCH_TELE2     4

#define OE_PIN           5   

uint16_t                positionsClosed[NUM_SERVOS] = {};
uint16_t                positionsOpen[NUM_SERVOS]   = {};
uint16_t                positionsSet[NUM_SERVOS]    = {};
uint8_t                 telescopeState[3]           = {0, 0, 0}; 
Adafruit_PWMServoDriver pwm                         = Adafruit_PWMServoDriver();
CmdMessenger            cmdMessenger                = CmdMessenger(Serial, ',', ';'); 

enum{                //      Closed     Open
  kAcknowledge,      // 0 
  kError,            // 1
  kDebug,            // 2
  kSetServo,         // 3   3,1,150;     3,1,400;
  kStoreServo,       // 4   4,1,0;       4,1,1;
  kStateChange,      // 5
  kTelescopePosition // 6   6,0,0;       6,0,1;    6,0,2;
};

void storePositions(){
  uint8_t  ee = 0;
  uint8_t* p  = (uint8_t*)(void*)&positionsClosed;

  for (uint8_t i = 0; i < sizeof(positionsClosed); i++){
      EEPROM.write(ee++, *p++);
  }

  ee = 100;
  p  = (uint8_t*)(void*)&positionsOpen;

  for (uint8_t i = 0; i < sizeof(positionsOpen); i++){
      EEPROM.write(ee++, *p++);
  }
  
}

void loadPositions(){

  uint8_t  ee = 0;
  uint8_t* p  = (uint8_t*)(void*)&positionsClosed;

  for (uint8_t i = 0; i < sizeof(positionsClosed); i++){
    *p++ = EEPROM.read(ee++);
  }

  ee = 100;
  p  = (uint8_t*)(void*)&positionsOpen;

  for (uint8_t i = 0; i < sizeof(positionsOpen); i++){
    *p++ = EEPROM.read(ee++);
  }

  // Middel state, we don't know jet
  for (uint8_t i = 0; i < NUM_SERVOS; i++){
    positionsSet[i] = 300; 
  }  

}

void OnSetServo(){
    cmdMessenger.sendCmd(kDebug, "SetServo called");
    uint8_t  servo      = cmdMessenger.readInt16Arg(); 
    uint16_t position   = cmdMessenger.readInt16Arg();
    positionsSet[servo] = position;
    pwm.setPWM(servo, 0, position);
    cmdMessenger.sendCmd(kAcknowledge, "StoreServo moved");
}

void OnStoreServo(){
    cmdMessenger.sendCmd(kDebug, "StoreServo called");
    uint8_t servo = cmdMessenger.readInt16Arg();
    uint8_t state = cmdMessenger.readInt16Arg();

    if (state == 0) {
      positionsClosed[servo] = positionsSet[servo];
    }

    if (state == 1) {
      positionsOpen[servo] = positionsSet[servo];    
    }

    storePositions();
    cmdMessenger.sendCmd(kAcknowledge, "StoreServo stored");
}

void moveToState(uint8_t telescope, uint8_t state){

  if (telescope == 0) {

    if (state == 0){                        // Close
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(1, 0, positionsOpen[1]);   // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsClosed[0]); // Close Cover 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(1, 0, positionsClosed[1]); // Close Bahtinov 
      telescopeState[0] = 0;
      
    } else if (state == 1){                 // Open
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(1, 0, positionsOpen[1]);   // Open Bahtinov
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsOpen[0]);   // Open Cover
      telescopeState[0] = 1;

    } else if (state == 2) {                // Bahtinov
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(1, 0, positionsOpen[1]);   // Open Bahtinov make sure its away
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsOpen[0]);   // Open cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(1, 0, positionsClosed[1]); // Close Bahtinov 
      telescopeState[0] = 2;
    }

  }

  if (telescope == 1) {

    if (state == 0){                        // Close
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]);   // Open Bahtinov 
      pwm.setPWM(5, 0, positionsOpen[5]);   // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsClosed[2]); // Close Cover 
      pwm.setPWM(3, 0, positionsClosed[3]); // Close Cover 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(4, 0, positionsClosed[4]); // Close Bahtinov 
      pwm.setPWM(5, 0, positionsClosed[5]); // Close Bahtinov 
      telescopeState[1] = 0;

    } else if (state == 1){                // Open
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]);   // Open Bahtinov make sure its away
      pwm.setPWM(5, 0, positionsOpen[5]);   // Open Bahtinov make sure its away
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsOpen[2]);   // Open cover
      pwm.setPWM(3, 0, positionsOpen[3]);   // Open cover
      telescopeState[1] = 1;

    } else if (state == 2) {                // Bahtinov
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]);   // Open Bahtinov make sure its away
      pwm.setPWM(5, 0, positionsOpen[5]);   // Open Bahtinov make sure its away
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsOpen[2]);   // Open cover
      pwm.setPWM(3, 0, positionsOpen[3]);   // Open cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(4, 0, positionsClosed[4]); // Close Bahtinov 
      pwm.setPWM(5, 0, positionsClosed[5]); // Close Bahtinov 
      telescopeState[1] = 2;
    }

  }

  if (telescope == 2) {

    if (state == 0){                        // Close
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]);   // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsClosed[6]); // Close Cover 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(7, 0, positionsClosed[7]); // Close Bahtinov 
      telescopeState[2] = 0;

    } else if (state == 1){                 // Open
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]);   // Open Bahtinov make sure its away
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsOpen[6]);   // Open Cover
      telescopeState[2] = 1;

    } else if (state == 2) {                // Bahtinow
      cmdMessenger.sendCmdStart(kStateChange);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]);   // Open Bahtinov make sure its away
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsOpen[6]);   // Open cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(7, 0, positionsClosed[7]); // Close Bahtinov 
      telescopeState[2] = 2;
    }
  }

}

void OnTelescopePosition(){
    cmdMessenger.sendCmd(kDebug, "move to position");
    uint8_t telescope = cmdMessenger.readInt16Arg();
       uint8_t state     = cmdMessenger.readInt16Arg();
       moveToState(telescope, state);
       cmdMessenger.sendCmd(kAcknowledge, "position reached");
}   

void attachCommandCallbacks(   ){
  cmdMessenger.attach(kSetServo,          OnSetServo);
  cmdMessenger.attach(kStoreServo,        OnStoreServo);
  cmdMessenger.attach(kTelescopePosition, OnTelescopePosition);
}

void setup() {
  pinMode(SWITCH_TELE0, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE2, INPUT_PULLUP);
  pinMode(OE_PIN,       OUTPUT);
  digitalWrite(OE_PIN,  HIGH); // disable

  Serial.begin(9600);
  cmdMessenger.printLfCr(true);
  attachCommandCallbacks();
  cmdMessenger.sendCmd(kDebug, "Arduino ready!");

  loadPositions();

  pwm.begin();  
  pwm.setPWMFreq(60);        // Analog servos run at ~60 Hz updates
  digitalWrite(OE_PIN, LOW); // enable

}

void loop() {  
  cmdMessenger.feedinSerialData();

  if(!digitalRead(SWITCH_TELE0)){
    moveToState(0, (telescopeState[0] + 1) % 3);
  }

  if(!digitalRead(SWITCH_TELE1)){
    moveToState(1, (telescopeState[1] + 1) % 3);
  }

  if(!digitalRead(SWITCH_TELE2)){
    moveToState(2, (telescopeState[2] + 1) % 3); 
  }

}
