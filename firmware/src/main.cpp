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
  kAcknowledge, // 0 
  kError,       // 1
};

void storePositions(){
  for(uint8_t i = 0; i < size(positions); i++){
    EEPROM.write(i,positions[i]);
  }
}

void readPositions(){

}

void OnUnknownCommand(){
  cmdMessenger.sendCmd(kError,"Command without attached callback");
}

void attachCommandCallbacks(){
  cmdMessenger.attach(OnUnknownCommand);
}

void setup() {
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);
  pinMode(SWITCH_TELE1, INPUT_PULLUP);

  EEPROM.write(0,0);



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
