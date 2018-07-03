#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <CmdMessenger.h>
#include <Adafruit_PWMServoDriver.h>

#define NUM_SERVOS       8
#define MOVE_WAIT_TIME 500

#define SWITCH_TELE1     2 
#define SWITCH_TELE2     3   
#define SWITCH_TELE3     4

#define OE_PIN           5   

uint16_t                positionsClosed[NUM_SERVOS] = {};
uint16_t                positionsOpen[NUM_SERVOS]   = {};
uint16_t                positionsSet[NUM_SERVOS]    = {};
uint8_t                 telescopeState[3]           = {0, 0, 0}; 
Adafruit_PWMServoDriver pwm                         = Adafruit_PWMServoDriver();
CmdMessenger            cmdMessenger                = CmdMessenger(Serial, ',', ';'); 

enum{                   // Closed     Open
  kAcknowledge,   // 0 
  kError,         // 1
  kDebug,         // 2
  kSetServo,      // 3   3,1,240;     3,1,480;
  kStoreServo,    // 4   4,1,0;       4,1,1;
  kButtonPressed, // 5
  kServoPosition  // 6   6,1,0;       6,1,1;
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
    uint8_t servo     = cmdMessenger.readInt16Arg(); 
    uint16_t position = cmdMessenger.readInt16Arg();
    positionsSet[servo] = position;
    pwm.setPWM(servo, 0, position);
    cmdMessenger.sendCmd(kDebug, "StoreServo moved");
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
    cmdMessenger.sendCmd(kDebug, "StoreServo stored");
}

void OnServoPosition(){
    cmdMessenger.sendCmd(kDebug, "move to position");
    uint8_t servo = cmdMessenger.readInt16Arg();
    uint8_t state = cmdMessenger.readInt16Arg();

    if (state == 0) {
      pwm.setPWM(servo, 0, positionsClosed[servo]);
    }

    if (state == 1) {
      pwm.setPWM(servo, 0, positionsOpen[servo]);
    }

    cmdMessenger.sendCmd(kDebug, "position reached");
}

void attachCommandCallbacks(){
  cmdMessenger.attach(kSetServo,      OnSetServo);
  cmdMessenger.attach(kStoreServo,    OnStoreServo);
  cmdMessenger.attach(kServoPosition, OnServoPosition);
}

void setup() {
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE2, INPUT_PULLUP);
  pinMode(SWITCH_TELE3, INPUT_PULLUP);
  pinMode(OE_PIN,       OUTPUT);
  digitalWrite(OE_PIN,  HIGH); // disable

  Serial.begin(9600);
  cmdMessenger.printLfCr(true);
  attachCommandCallbacks();
  cmdMessenger.sendCmd(kDebug, "Arduino ready!");

  loadPositions();

  pwm.begin();  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  for (uint8_t i = 0; i < NUM_SERVOS; i++){
    pwm.setPWM(i, 0, positionsClosed[i]);
  }

  digitalWrite(OE_PIN, LOW); // enable

}

void loop() {  
  cmdMessenger.feedinSerialData();

  if(!digitalRead(SWITCH_TELE1)){
    
    if (telescopeState[0] == 0){          // Closed -> Open
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(1, 0, positionsOpen[1]); // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsOpen[0]); // Open Cover 
      telescopeState[0] = 1;

    } else if (telescopeState[0] == 1){     // Open -> Bahtinov
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      cmdMessenger.sendCmd(kButtonPressed, 1);
      pwm.setPWM(1, 0, positionsOpen[1]);   // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsOpen[0]);   // Open Cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(1, 0, positionsClosed[1]); // Open Bahtinov
      telescopeState[0] = 2;

    } else if (telescopeState[0] == 2) {    // Bahtinov -> Close
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      cmdMessenger.sendCmd(kButtonPressed, 1);
      pwm.setPWM(1, 0, positionsOpen[1]);   // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(0, 0, positionsClosed[0]); // Close cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(1, 0, positionsClosed[1]); // Close Bahtinov 
      telescopeState[0] = 0;
    }

  }

  if(!digitalRead(SWITCH_TELE2)){

    if (telescopeState[1] == 0){          // Closed -> Open
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]); // Open Bahtinov 
      pwm.setPWM(5, 0, positionsOpen[5]); // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsOpen[2]); // Open Cover 
      pwm.setPWM(3, 0, positionsOpen[3]); // Open Cover 
      telescopeState[1] = 1;

    } else if (telescopeState[1] == 1){    // Open -> Bahtinov
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]);  // Open Bahtinov make sure its avay
      pwm.setPWM(5, 0, positionsOpen[5]);  // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsOpen[2]);  // Open cover
      pwm.setPWM(3, 0, positionsOpen[3]);  // Open cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(4, 0, positionsClosed[4]); // Open Bahtinov
      pwm.setPWM(5, 0, positionsClosed[5]); // Open Bahtinov
      telescopeState[1] = 2;

    } else if (telescopeState[1] == 2) {   // Bahtinov -> Close
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(4, 0, positionsOpen[4]);  // Open Bahtinov make sure its avay
      pwm.setPWM(5, 0, positionsOpen[5]);  // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(2, 0, positionsClosed[2]); // Close cover
      pwm.setPWM(3, 0, positionsClosed[3]); // Close cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(4, 0, positionsClosed[4]); // Close Bahtinov 
      pwm.setPWM(5, 0, positionsClosed[5]); // Close Bahtinov 
      telescopeState[1] = 0;
    }

  }

  if(!digitalRead(SWITCH_TELE3)){
  
    if (telescopeState[2] == 0){        // Closed -> Open
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(1);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]); // Open Bahtinov 
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsOpen[6]); // Open Cover 
      telescopeState[2] = 1;

    } else if (telescopeState[2] == 1){     // Open -> Bahtinov
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]);   // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsOpen[6]);   // Open Cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(7, 0, positionsClosed[7]); // Close Bahtinov
      telescopeState[2] = 2;

    } else if (telescopeState[2] == 2) {    // Bahtinow -> Close
      cmdMessenger.sendCmdStart(kButtonPressed);  
      cmdMessenger.sendCmdArg(2);    
      cmdMessenger.sendCmdArg(0);    
      cmdMessenger.sendCmdEnd();
      pwm.setPWM(7, 0, positionsOpen[7]);   // Open Bahtinov make sure its avay
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(6, 0, positionsClosed[6]); // Close cover
      delay(MOVE_WAIT_TIME);
      pwm.setPWM(7, 0, positionsClosed[7]); // Close Bahtinov 
      telescopeState[2] = 0;
    }
  
  }

}
