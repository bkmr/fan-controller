// https://gist.github.com/EEVblog/6206934

#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "room.h"

#define bedOne 8
#define bedTwo 10
#define bedThree 12
#define study 14
#define pwmLed 11

#define IRLEDpin  12              //the arduino pin connected to IR LED to ground. HIGH=LED ON
#define BITtime   397            //length of the carrier bit in microseconds
#define Setuptime   1640            //length of the carrier bit in microseconds

static int address = bedOne;
static boolean test;
static boolean testIn;

static SwitchLed switchOff = SwitchLed(0, 1);
static SwitchLed switchLow = SwitchLed(2, 3);
static SwitchLed switchMedium = SwitchLed(4, 5);
static SwitchLed switchHigh = SwitchLed(6, 7);

static SwitchLed switchOne = SwitchLed(8, 9);
static SwitchLed switchTwo = SwitchLed(10, 11);
static SwitchLed switchThree = SwitchLed(12, 13);
static SwitchLed switchFour = SwitchLed(14, 15);

// high nibble is DIP switch address on units
#define ADDR_BED_1 0b1110
#define ADDR_BED_2 0b1010
#define ADDR_BED_3 0b1011
#define ADDR_STUDY 0b1101

static Room roomOne = Room(switchOne, ADDR_BED_1);
static Room roomTwo = Room(switchTwo, ADDR_BED_2);
static Room roomThree = Room(switchThree, ADDR_BED_3);
static Room roomFour = Room(switchFour, ADDR_STUDY);

static Room* selectedRoom = &roomOne;

Adafruit_MCP23017 mcp;
static uint16_t mcpInputs;
static uint16_t mcpOutputs;

unsigned char codeBase[] = {0xA3, 0x16, 0x09, 0x00};

void setup()
{
  Serial.begin(9600);
  pinMode(pwmLed, OUTPUT);
  
  pinMode(IRLEDpin, OUTPUT);

  digitalWrite(IRLEDpin, LOW);    //turn off IR LED to start

  mcp.begin();
  mcp.pinMode(switchOff.switchPin(), INPUT);
  mcp.pullUp(switchOff.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchLow.switchPin(), INPUT);
  mcp.pullUp(switchLow.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchMedium.switchPin(), INPUT);
  mcp.pullUp(switchMedium.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchHigh.switchPin(), INPUT);
  mcp.pullUp(switchHigh.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchOne.switchPin(), INPUT);
  mcp.pullUp(switchOne.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchTwo.switchPin(), INPUT);
  mcp.pullUp(switchTwo.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchThree.switchPin(), INPUT);
  mcp.pullUp(switchThree.switchPin(), HIGH);  // turn on a 100K pullup internally
  mcp.pinMode(switchFour.switchPin(), INPUT);
  mcp.pullUp(switchFour.switchPin(), HIGH);  // turn on a 100K pullup internally

  mcp.pinMode(switchOff.ledPin(), OUTPUT);
  mcp.pinMode(switchLow.ledPin(), OUTPUT);
  mcp.pinMode(switchMedium.ledPin(), OUTPUT);
  mcp.pinMode(switchHigh.ledPin(), OUTPUT);
  mcp.pinMode(switchOne.ledPin(), OUTPUT);
  mcp.pinMode(switchTwo.ledPin(), OUTPUT);
  mcp.pinMode(switchThree.ledPin(), OUTPUT);
  mcp.pinMode(switchFour.ledPin(), OUTPUT);
  
  // turn on LEDs
  mcp.digitalWrite(switchOff.ledPin(), HIGH);
  mcp.digitalWrite(switchLow.ledPin(), HIGH);
  mcp.digitalWrite(switchMedium.ledPin(), HIGH);
  mcp.digitalWrite(switchHigh.ledPin(), HIGH);
  mcp.digitalWrite(switchOne.ledPin(), HIGH);
  mcp.digitalWrite(switchTwo.ledPin(), HIGH);
  mcp.digitalWrite(switchThree.ledPin(), HIGH);
  mcp.digitalWrite(switchFour.ledPin(), HIGH);
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

unsigned long getCommand(unsigned char command, unsigned char address) {
  unsigned char addressShift = address << 4;
  unsigned char code[4];
  code[0] = codeBase[0];
  code[1] = codeBase[1];
  code[2] = codeBase[2] + addressShift;
  code[3] = command;
  return commandArrayToTransmit(code);
}

void loop()
{
  // get inputs
  mcpInputs = mcp.readGPIOAB();
  
  auto one =   !(mcpInputs & 0x1 << switchOne.switchPin());
  auto two =   !(mcpInputs & 0x1 << switchTwo.switchPin());
  auto three = !(mcpInputs & 0x1 << switchThree.switchPin());
  auto four =  !(mcpInputs & 0x1 << switchFour.switchPin());
  if(one) selectedRoom = &roomOne;
  if(two) selectedRoom = &roomTwo;
  if(three) selectedRoom = &roomThree;
  if(four) selectedRoom = &roomFour;
  
  auto off =    !(mcpInputs & 0x1 << switchOff.switchPin());
  auto low =    !(mcpInputs & 0x1 << switchLow.switchPin());
  auto medium = !(mcpInputs & 0x1 << switchMedium.switchPin());
  auto high =   !(mcpInputs & 0x1 << switchHigh.switchPin());
  // check for change of fan state
  auto fanSelectActive = off || low || medium || high;
  if(fanSelectActive) {
    auto fanSpeed = selectedRoom->fanSpeed;
    auto newFan = (fanSpeed == Room::FanSpeed::FAN_OFF && !off) ||
                  (fanSpeed == Room::FanSpeed::FAN_LOW && !low) ||
                  (fanSpeed == Room::FanSpeed::FAN_MEDIUM && !medium) ||
                  (fanSpeed == Room::FanSpeed::FAN_HIGH && !high);
    if(newFan) {
      if(off)    selectedRoom->fanSpeed = Room::FanSpeed::FAN_OFF;
      if(low)    selectedRoom->fanSpeed = Room::FanSpeed::FAN_LOW;
      if(medium) selectedRoom->fanSpeed = Room::FanSpeed::FAN_MEDIUM;
      if(high)   selectedRoom->fanSpeed = Room::FanSpeed::FAN_HIGH;

      // write fanSpeed to RF
      unsigned long command;
      switch(selectedRoom->fanSpeed) {
      case Room::FanSpeed::FAN_OFF:
        command = getCommand(CMD_OFF, selectedRoom->address());
        break;
      case Room::FanSpeed::FAN_LOW:
        command = getCommand(CMD_LOW, selectedRoom->address());
        break;
      case Room::FanSpeed::FAN_MEDIUM:
        command = getCommand(CMD_MEDIUM, selectedRoom->address());
        break;
      case Room::FanSpeed::FAN_HIGH:
        command = getCommand(CMD_HIGH, selectedRoom->address());
        break;
      default:
        command = getCommand(CMD_OFF, selectedRoom->address());
        break;
      }
      IRSendCode(command);
    }
  }
  
  // write output leds
  mcpOutputs = 0;
  if(selectedRoom == &roomOne) mcpOutputs += 0x1 << switchOne.ledPin();
  if(selectedRoom == &roomTwo) mcpOutputs += 0x1 << switchTwo.ledPin();
  if(selectedRoom == &roomThree) mcpOutputs += 0x1 << switchThree.ledPin();
  if(selectedRoom == &roomFour) mcpOutputs += 0x1 << switchFour.ledPin();
  mcpOutputs += 0x1 << switchOff.ledPin();
  mcpOutputs += 0x1 << switchLow.ledPin();
  mcpOutputs += 0x1 << switchMedium.ledPin();
  mcpOutputs += 0x1 << switchHigh.ledPin();
  mcp.writeGPIOAB(mcpOutputs);
  

//  printSerialDebug();
}


void printSerialDebug() {
//  Serial.print(!digitalRead(bedOne));
//  Serial.print(!digitalRead(bedTwo));
//  Serial.print(!digitalRead(bedThree));
////  Serial.print(!digitalRead(study));
//  Serial.print(" ");
//  Serial.print(!digitalRead(offIn));
//  Serial.print(!digitalRead(lowIn));
//  Serial.print(!digitalRead(mediumIn));
//  Serial.print(!digitalRead(highIn));
  Serial.print(mcpInputs, BIN);
  Serial.print(" ");
  Serial.print(mcpOutputs, BIN);
  Serial.println();
}

