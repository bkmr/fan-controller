// https://gist.github.com/EEVblog/6206934

#include <Wire.h>
#include "Adafruit_MCP23017.h"

#define offIn A0
#define lowIn A1
#define mediumIn A2
#define highIn A3

#define bedOne 4
#define bedTwo 5
#define bedThree 6
#define study 7

#define bedOneLed 8
#define bedTwoLed 9
#define bedThreeLed 10
#define studyLed 7
#define pwmLed 11

#define IRLEDpin  12              //the arduino pin connected to IR LED to ground. HIGH=LED ON
#define BITtime   397            //length of the carrier bit in microseconds
#define Setuptime   1640            //length of the carrier bit in microseconds

static int address = bedOne;

unsigned char codeBase[] = {0xA3, 0x16, 0x09, 0x00};
Adafruit_MCP23017 mcp;

void setup()
{
  Serial.begin(9600);
  pinMode(offIn, INPUT_PULLUP);
  pinMode(lowIn, INPUT_PULLUP);
  pinMode(mediumIn, INPUT_PULLUP);
  pinMode(highIn, INPUT_PULLUP);

  pinMode(bedOne, INPUT_PULLUP);
  pinMode(bedTwo, INPUT_PULLUP);
  pinMode(bedThree, INPUT_PULLUP);
//  pinMode(study, INPUT_PULLUP);

  pinMode(bedOneLed, OUTPUT);
  pinMode(bedTwoLed, OUTPUT);
  pinMode(bedThreeLed, OUTPUT);
  pinMode(studyLed, OUTPUT);
  pinMode(pwmLed, OUTPUT);
  
  pinMode(IRLEDpin, OUTPUT);

  digitalWrite(IRLEDpin, LOW);    //turn off IR LED to start
  digitalWrite(bedOneLed, LOW);
  digitalWrite(bedTwoLed, LOW);
  digitalWrite(bedThreeLed, LOW);
  digitalWrite(studyLed, LOW);

  mcp.begin();
  mcp.pinMode(7, INPUT);
  mcp.pullUp(7, HIGH);  // turn on a 100K pullup internally

  mcp.pinMode(8, OUTPUT);

  analogWrite(pwmLed, 240);

  delay(250);
}

void WriteBit(bool high)
{
  auto highTime = high ? BITtime * 2 : BITtime;
  digitalWrite(IRLEDpin, HIGH);
  delayMicroseconds(highTime);
  digitalWrite(IRLEDpin, LOW);
  delayMicroseconds(BITtime);
}

void IRSendCode(unsigned long code)
{
  auto codeCache = code;
  // set carrier
  digitalWrite(IRLEDpin, HIGH);
  delayMicroseconds(BITtime);

  for(int l = 0; l < 10; l++) {
    codeCache = code;
    
    // send start sequence LOW HIGH LOW
    digitalWrite(IRLEDpin, LOW);
    delayMicroseconds(Setuptime);
    digitalWrite(IRLEDpin, HIGH);
    delayMicroseconds(Setuptime);
    digitalWrite(IRLEDpin, LOW);
    delayMicroseconds(BITtime);
    
    //send the user defined 4 byte/32bit code
    // MSB encoded, because WTF?
    for (int i=32; i>0; i--)            //send all 4 bytes or 32 bits
    {
      WriteBit(codeCache & 0x1);            //get the current bit by masking all but the MSB
      codeCache >>= 1;
    }
   }
}

unsigned long commandArrayToTransmit(unsigned char cmd[]) {
  unsigned long code =  cmd[3] * (1L << 24);
                code += cmd[2] * (1L << 16);
                code += cmd[1] * (1L << 8);
                code += cmd[0] * (1L << 0);
    return code;
}

#define CMD_OFF    0x14
#define CMD_LOW    0x18
#define CMD_MEDIUM 0x10
#define CMD_HIGH   0x11

// high nibble is DIP switch address on units
#define ADDR_BED_1 0b1110
#define ADDR_BED_2 0b1010
#define ADDR_BED_3 0b1011
#define ADDR_STUDY 0b1101

unsigned long getCommand(unsigned char command, unsigned char address) {
  unsigned char addressShift = address << 4;
  unsigned char code[4];
  code[0] = codeBase[0];
  code[1] = codeBase[1];
  code[2] = codeBase[2] + addressShift;
  code[3] = command;
  return commandArrayToTransmit(code);
}

void loop()                           //some demo main code
{
  auto off = !digitalRead(offIn);
  auto low = !digitalRead(lowIn);
  auto medium = !digitalRead(mediumIn);
  auto high = !digitalRead(highIn);

  if (!digitalRead(bedOne)) {
    address = bedOne; 
  } 
  if (!digitalRead(bedTwo)) {
    address = bedTwo; 
  } 
  if (!digitalRead(bedThree)) {
    address = bedThree; 
  } 
//  if (!digitalRead(study)) {
//    address = study; 
//  } 
  if(!mcp.digitalRead(7)) {
    address = study;
  }

  writeAddressToLeds(address);
  
  if(off) {
    unsigned long code = getCommand(CMD_OFF, getDipCodeForRoom(address));
    IRSendCode(code);
  }
  else if(low) {
    unsigned long code = getCommand(CMD_LOW, getDipCodeForRoom(address));
    IRSendCode(code);
  }
  else if(medium) {
    unsigned long code = getCommand(CMD_MEDIUM, getDipCodeForRoom(address));
    IRSendCode(code);
  }
  else if(high) {
    unsigned long code = getCommand(CMD_HIGH, getDipCodeForRoom(address));
    IRSendCode(code);
  }

//  digitalWrite(studyLed, mcp.digitalRead(7));

  printSerialDebug();

//  delay(100);
}

unsigned char getDipCodeForRoom(int addr) {
    switch(addr) {
  case bedOne:
    return ADDR_BED_1;
  case bedTwo:
    return ADDR_BED_2;
  case bedThree:
    return ADDR_BED_3;
  case study:
    return ADDR_STUDY;
  default:
    return 0x00;
  }
}

void writeAddressToLeds(int addr) {
  // set all LEDs OFF
  digitalWrite(bedOneLed, HIGH);
  digitalWrite(bedTwoLed, HIGH);
  digitalWrite(bedThreeLed, HIGH);
  digitalWrite(studyLed, HIGH);
  mcp.digitalWrite(8, HIGH);

  // write correct LED
  switch(addr) {
  case bedOne:
    digitalWrite(bedOneLed, LOW);
    break;
  case bedTwo:
    digitalWrite(bedTwoLed, LOW);
    break;
  case bedThree:
    digitalWrite(bedThreeLed, LOW);
    break;
  case study:
    digitalWrite(studyLed, LOW);
    mcp.digitalWrite(8, LOW);
    break;
  default:
    break;
  }
}

void printSerialDebug() {
  Serial.print(!digitalRead(bedOne));
  Serial.print(!digitalRead(bedTwo));
  Serial.print(!digitalRead(bedThree));
//  Serial.print(!digitalRead(study));
  Serial.print(" ");
  Serial.print(!digitalRead(offIn));
  Serial.print(!digitalRead(lowIn));
  Serial.print(!digitalRead(mediumIn));
  Serial.print(!digitalRead(highIn));
  Serial.println();
}

