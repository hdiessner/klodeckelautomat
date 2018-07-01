#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <CmdMessenger.h>
#include <Adafruit_PWMServoDriver.h>

#define SWITCH_TELE1 2 
#define SWITCH_TELE2 3   
#define SWITCH_TELE3 4
#define OE_PIN       5   

uint8_t                 positions[16] = {0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255};
Adafruit_PWMServoDriver pwm           = Adafruit_PWMServoDriver();
CmdMessenger            cmdMessenger  = CmdMessenger(Serial, ',', ';'); 

enum{
  kAcknowledge,   // 0 
  kError,         // 1
  kSetServo,      // 2
  kStoreServo,    // 3
  kButtonPressed, // 4
  kServoPosition  // 5
};

void storePositions(){
  for(uint8_t i = 0; i < sizeof(positions); i++){
    EEPROM.write(i, positions[i]);
  }
}

void loadPositions(){
  for(uint8_t i = 0; i < sizeof(positions); i++){
    positions[i] = EEPROM.read(i);
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

  loadPositions();

  Serial.begin(9600);
  cmdMessenger.printLfCr(true);
  attachCommandCallbacks();
  cmdMessenger.sendCmd(kAcknowledge, "Arduino ready!");
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
