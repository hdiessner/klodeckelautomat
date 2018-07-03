#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <CmdMessenger.h>
#include <Adafruit_PWMServoDriver.h>

#define NUM_SERVOS   8

#define SWITCH_TELE1 2 
#define SWITCH_TELE2 3   
#define SWITCH_TELE3 4
#define OE_PIN       5   

uint16_t                positionsMin[NUM_SERVOS] = {};
uint16_t                positionsMax[NUM_SERVOS] = {};
Adafruit_PWMServoDriver pwm                      = Adafruit_PWMServoDriver();
CmdMessenger            cmdMessenger             = CmdMessenger(Serial, ',', ';'); 

enum{
  kAcknowledge,   // 0 
  kError,         // 1
  kSetServo,      // 2
  kStoreServo,    // 3
  kButtonPressed, // 4
  kServoPosition  // 5
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

}

void OnUnknownCommand(){
  cmdMessenger.sendCmd(kError,"Command without attached callback");
}

void OnSetServo(){

}

void OnStoreServo(){

}

void OnButtonPressed(){

}

void OnServoPosition(){

}

void attachCommandCallbacks(){
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(OnSetServo);
  cmdMessenger.attach(OnStoreServo);
  cmdMessenger.attach(OnButtonPressed);
  cmdMessenger.attach(OnServoPosition);
}

void setup() {
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(OE_PIN      , OUTPUT);
  digitalWrite(OE_PIN, LOW); // Enable

  Serial.begin(9600);
  cmdMessenger.printLfCr(true);
  attachCommandCallbacks();
  cmdMessenger.sendCmd(kAcknowledge, "Arduino ready!");

  loadPositions();

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
