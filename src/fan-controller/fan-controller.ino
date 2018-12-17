// https://gist.github.com/EEVblog/6206934

#define offIn 8
#define lowIn 9
#define mediumIn 10
#define highIn 11

#define IRLEDpin  3              //the arduino pin connected to IR LED to ground. HIGH=LED ON
#define BITtime   397            //length of the carrier bit in microseconds
#define Setuptime   1640            //length of the carrier bit in microseconds
//put your own code here - 4 bytes (ADDR1 | ADDR2 | COMMAND1 | COMMAND2)
//unsigned long IRcode=0b11000001110001111100000000111111;  
unsigned long   codeOff=   0b00010100110110010001011010100011;  // study off
unsigned long   codeLow=   0b00011000110110010001011010100011;  // study low
unsigned long   codeMedium=0b00010000110110010001011010100011;  // study medium
unsigned long   codeHigh=  0b00010001110110010001011010100011;  // study high
//unsigned long   codeOff=   0b00010100111010010001011010100011;  // bed1 off
//unsigned long   codeLow=   0b00011000111010010001011010100011;  // bed1 low
//unsigned long   codeMedium=0b00010000111010010001011010100011;  // bed1 medium
//unsigned long   codeHigh=  0b00010001111010010001011010100011;  // bed1 high

// SOME CODES:
// Canon WL-D89 video remote START/STOP button = 0b11000001110001111100000000111111

void setup()
{
  IRsetup();                          //Only need to call this once to setup
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

//Sends the IR code in 4 byte NEC format
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
    for (int i=32; i>0; i--)            //send all 4 bytes or 32 bits
    {
      WriteBit(codeCache & 0x1);            //get the current bit by masking all but the MSB
      codeCache >>= 1;
    }
   }
}

void loop()                           //some demo main code
{
  auto off = !digitalRead(offIn);
  auto low = !digitalRead(lowIn);
  auto medium = !digitalRead(mediumIn);
  auto high = !digitalRead(highIn);
  
  if(off) {
    IRSendCode(codeOff);
  }
  else if(low) {
    IRSendCode(codeLow);
  }
  else if(medium) {
    IRSendCode(codeMedium);
  }
  else if(high) {
    IRSendCode(codeHigh);
  }
}
