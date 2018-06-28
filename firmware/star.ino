#include <Servo.h>
#include <CmdMessenger.h>

#define S1PIN           3
#define S2PIN           9
#define S3PIN           6
#define S4PIN           7
#define SWITCH1PIN      8
#define SWITCH2PIN      4
#define SWITCH3PIN      5

#define LIGHTPIN        2 // Internal Used by frequency counter
#define LIGHTDELAY 300000

Servo             myservo;
CmdMessenger      cmdMessenger     = CmdMessenger(Serial, ',', ';'); 
uint32_t          lcdcounter       = 1000000000;
bool              shutterclosed[4] = {true, true, true, true};
uint8_t           openval[4]       = {100, 100, 100, 100};
uint8_t           closeval[4]      = { 30,  10,  30,  30};
uint8_t           pins[4]          = {S1PIN, S2PIN, S3PIN, S4PIN};
volatile uint32_t lightCounter     = 0;

enum{
  kAcknowledge, // 0 
  kError,       // 1
  kLDR,         // 2
  kS1,          // 3
  kS2,          // 4
  kS3,          // 5
  kS4,          // 6
  kSOpen,       // 7
  kSClose,      // 8
};

void OnS1(){
  int8_t val = cmdMessenger.readInt16Arg();
  setServo(0, val );
}

void OnS2(){
  int8_t val = cmdMessenger.readInt16Arg();
  setServo(1, val);  
}

void OnS3(){
  int8_t val = cmdMessenger.readInt16Arg();
  setServo(2, val);
}

void OnS4(){
  int8_t val = cmdMessenger.readInt16Arg();
  setServo(3, val);
}

void attachCommandCallbacks(){
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kS1, OnS1);
  cmdMessenger.attach(kS2, OnS2);
  cmdMessenger.attach(kS3, OnS3);
  cmdMessenger.attach(kS4, OnS4);
}

void OnUnknownCommand(){
  cmdMessenger.sendCmd(kError,"Command without attached callback");
}

void lightTick(){
  lightCounter++;
}

uint16_t getLDR(void){
  lightCounter = 0;
  attachInterrupt(digitalPinToInterrupt(LIGHTPIN), lightTick, RISING);  
  delay(1000);
  detachInterrupt(digitalPinToInterrupt(LIGHTPIN));
  return lightCounter;
}

void setServo(uint8_t pin, uint8_t pos){  
  if (pos > 45){
    openval[pin] = pos;
    shutterclosed[pin] = false;
    cmdMessenger.sendCmd(kSOpen, pin);
  }else{
    closeval[pin] = pos;
    shutterclosed[pin]  = true;
    cmdMessenger.sendCmd(kSClose, pin);
  }
  myservo.attach(pins[pin]);  
  myservo.write(pos);    
  delay(1500);
  pinMode(pins[pin], INPUT);  
}

void setup() {
  pinMode(S1PIN, INPUT);
  pinMode(S2PIN, INPUT);
  pinMode(S3PIN, INPUT);
  pinMode(S4PIN, INPUT);

  pinMode(SWITCH1PIN, INPUT_PULLUP);
  pinMode(SWITCH2PIN, INPUT_PULLUP);
  pinMode(SWITCH3PIN, INPUT_PULLUP);

  Serial.begin(9600);
  cmdMessenger.printLfCr(true);
  attachCommandCallbacks();
  cmdMessenger.sendCmd(kAcknowledge, "Arduino ready!");

}

void loop() {  
  cmdMessenger.feedinSerialData();

  if(!digitalRead(SWITCH1PIN)){
    if(shutterclosed[0]){
      setServo(0, openval[0]);
      shutterclosed[0] = false;
    }else{
      setServo(0, closeval[0]);
      shutterclosed[0] = true;      
    }
  }

  if(!digitalRead(SWITCH2PIN)){
    if(shutterclosed[1]){
      setServo(1, openval[1]);
      shutterclosed[1] = false;
    }else{
      setServo(1, closeval[1]);
      shutterclosed[1] = true;      
    }    
  }

  if(!digitalRead(SWITCH3PIN)){
    if(shutterclosed[2]){
      setServo(2, openval[2]);
      shutterclosed[2] = false;
    }else{
      setServo(2, closeval[2]);
      shutterclosed[2] = true;      
    }
    if(shutterclosed[3]){
      setServo(3, openval[3]);
      shutterclosed[3] = false;
    }else{
      setServo(3, closeval[3]);
      shutterclosed[3] = true;      
    }
  }

  if(lcdcounter++ > LIGHTDELAY){
    cmdMessenger.sendCmd(kLDR, getLDR());
    lcdcounter = 0;
  }
  
}
