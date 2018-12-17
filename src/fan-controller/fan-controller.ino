// https://gist.github.com/EEVblog/6206934

#define offIn 8
#define lowIn 9
#define mediumIn 10
#define highIn 11

#define IRLEDpin  3              //the arduino pin connected to IR LED to ground. HIGH=LED ON
#define BITtime   397            //length of the carrier bit in microseconds
#define Setuptime   1640            //length of the carrier bit in microseconds

unsigned char codeBase[] = {0xA3, 0x16, 0x09, 0x00};

void setup()
{
  IRsetup();                          //Only need to call this once to setup
//  Serial.begin(115200);
}

void IRsetup(void)
{
  pinMode(offIn, INPUT_PULLUP);
  pinMode(lowIn, INPUT_PULLUP);
  pinMode(mediumIn, INPUT_PULLUP);
  pinMode(highIn, INPUT_PULLUP);
  pinMode(IRLEDpin, OUTPUT);
  digitalWrite(IRLEDpin, LOW);    //turn off IR LED to start
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
  
  if(off) {
    unsigned long code = getCommand(CMD_OFF, ADDR_STUDY);
    IRSendCode(code);
  }
  else if(low) {
    unsigned long code = getCommand(CMD_LOW, ADDR_STUDY);
    IRSendCode(code);
  }
  else if(medium) {
    unsigned long code = getCommand(CMD_MEDIUM, ADDR_STUDY);
    IRSendCode(code);
  }
  else if(high) {
    unsigned long code = getCommand(CMD_HIGH, ADDR_STUDY);
    IRSendCode(code);
  }
}
