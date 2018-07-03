#include <Wire.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <CmdMessenger.h>
#include <Adafruit_PWMServoDriver.h>

#define NUM_SERVOS   8

#define SWITCH_TELE1 2 
#define SWITCH_TELE2 3   
#define SWITCH_TELE3 4
#define OE_PIN       5   

uint16_t                positionsMin[NUM_SERVOS] = {};
uint16_t                positionsMax[NUM_SERVOS] = {};
uint16_t                positionsSet[NUM_SERVOS] = {};
bool                    positionsNow[NUM_SERVOS] = {};
Adafruit_PWMServoDriver pwm                      = Adafruit_PWMServoDriver();
CmdMessenger            cmdMessenger             = CmdMessenger(Serial, ',', ';'); 

enum{
  kAcknowledge,   // 0 
  kError,         // 1
  kDebug,         // 2
  kSetServo,      // 3   3,0,150;     3,0,570;
  kStoreServo,    // 4   4,0,0;       4,0,1;
  kButtonPressed, // 5
  kServoPosition  // 6   6,0,0;       6,0,1;
};

void storePositions(){
  uint8_t  ee = 0;
  uint8_t* p  = (uint8_t*)(void*)&positionsMin;

  for (uint8_t i = 0; i < sizeof(positionsMin); i++){
      EEPROM.write(ee++, *p++);
  }

  ee = 100;
  p  = (uint8_t*)(void*)&positionsMax;

  for (uint8_t i = 0; i < sizeof(positionsMax); i++){
      EEPROM.write(ee++, *p++);
  }
}

void loadPositions(){

  uint8_t  ee = 0;
  uint8_t* p  = (uint8_t*)(void*)&positionsMin;

  for (uint8_t i = 0; i < sizeof(positionsMin); i++){
    *p++ = EEPROM.read(ee++);
  }

  ee = 100;
  p  = (uint8_t*)(void*)&positionsMax;

  for (uint8_t i = 0; i < sizeof(positionsMax); i++){
    *p++ = EEPROM.read(ee++);
  }

  for (uint8_t i = 0; i < NUM_SERVOS; i++){
    positionsSet[i] = 500; 
  }  

}

void OnSetServo(){
    cmdMessenger.sendCmd(kDebug, "SetServo called");
    uint8_t servo    = cmdMessenger.readInt16Arg(); 
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
      positionsMin[servo] = positionsSet[servo];
    }

    if (state == 1) {
      positionsMax[servo] = positionsSet[servo];    
    }

    storePositions();
    cmdMessenger.sendCmd(kDebug, "StoreServo stored");
}

void OnServoPosition(){
    cmdMessenger.sendCmd(kDebug, "move to position");
    uint8_t servo = cmdMessenger.readInt16Arg();
    uint8_t state = cmdMessenger.readInt16Arg();

    if (state == 0) {
      pwm.setPWM(servo, 0, positionsMin[servo]);
    }

    if (state == 1) {
      pwm.setPWM(servo, 0, positionsMax[servo]);
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
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
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
    pwm.setPWM(i, 0, positionsMin[i]);
    positionsNow[i] = false;
  }

  digitalWrite(OE_PIN, LOW); // enable

}

void loop() {  
  cmdMessenger.feedinSerialData();

  if(!digitalRead(SWITCH_TELE1)){
  }

  if(!digitalRead(SWITCH_TELE2)){
  }

  if(!digitalRead(SWITCH_TELE3)){
  }

}
