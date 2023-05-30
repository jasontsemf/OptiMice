const int LEDPin = 14; 
const int OptiDataPin = 10;
const int OptiClkPin = 9;

const unsigned char LedOnStatus = 0x11;

//int correctPId1 = 0x31;
int correctPId1 = 48;
int cnt1 = 0;

const uint8_t OptiRegDx = 0x03; // 0x03
const uint8_t OptiRegDy = 0x04; // 0x04
const uint8_t OptiRegMotion = 0x02;
const uint8_t OptiRegProd1 =0x00;

int32_t optiX;
int32_t optiY;
signed char optiProd1;
signed char optiMotion;

char isOptiConnected = 0;

uint8_t OptiReadRegister(uint8_t address)
{
  int i = 7;
  uint8_t r = 0;
  // Write the address of the register we want to read:
  pinMode (OptiDataPin, OUTPUT);
  for (i=7; i>=0; i--)
  {
    digitalWrite (OptiClkPin, LOW);
    //delayMicroseconds(50);
    digitalWrite (OptiDataPin, address & (1 << i));
    delayMicroseconds(1);
    digitalWrite (OptiClkPin, HIGH);
    delayMicroseconds(5);
  }
  // Switch data line from OUTPUT to INPUT
  pinMode (OptiDataPin, INPUT);
  // Wait a bit...
  delayMicroseconds(30);
  // Fetch the data!
  for (i=7; i>=0; i--)
  {                             
    digitalWrite (OptiClkPin, LOW);
    delayMicroseconds(1);
    digitalWrite (OptiClkPin, HIGH);
    delayMicroseconds(4);
    r |= (digitalRead (OptiDataPin) << i);
    delayMicroseconds(1);
  }
  delayMicroseconds(25);
  return r;
}

void OptiWriteRegister(uint8_t address, uint8_t data)
{
  int i = 7;
  
  // Set MSB high, to indicate write operation:
  address |= 0x80;
  // Write the address:
  pinMode (OptiDataPin, OUTPUT);
  for (i=7; i>=0; i--)
  {
    digitalWrite (OptiClkPin, LOW);
    //delayMicroseconds(5);
    digitalWrite (OptiDataPin, address & (1 << i));
    delayMicroseconds(1);
    digitalWrite (OptiClkPin, HIGH);
    delayMicroseconds(5);
  }
  delayMicroseconds(30);
  // Write the data:
  for (i=7; i>=0; i--)
  {
    digitalWrite (OptiClkPin, LOW);
    //delayMicroseconds(5);
    digitalWrite (OptiDataPin, data & (1 << i));
    delayMicroseconds(1);
    digitalWrite (OptiClkPin, HIGH);
    delayMicroseconds(5);
  }
}

signed char OptiDx(){
  return  (signed char) OptiReadRegister(OptiRegDx);
}

signed char OptiDy(){
  return  (signed char) OptiReadRegister(OptiRegDy);
}

signed char OptiProductId1(){
  return  (signed char) OptiReadRegister(OptiRegProd1);
}

signed char OptiIsMotion(){
  signed char d = OptiReadRegister(OptiRegMotion);
  optiMotion = d;
  return optiMotion == -122 ? 1 : 0;
}

void OptiBegin(void)
{
  optiX = 0;
  optiY = 0;
  
  digitalWrite(OptiClkPin, HIGH);                     
  delayMicroseconds(5);
  digitalWrite(OptiClkPin, LOW);
  delayMicroseconds(1);
  digitalWrite(OptiClkPin, HIGH); 
  delay(100);
}

void OptiSetup(){
  int cnt = 0, PId1;
  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);
  Serial.println("OptiSetup");
  while(1){
  OptiBegin();
  PId1 = OptiProductId1();
  Serial.println(PId1);
  if( PId1 == correctPId1){
    isOptiConnected = 1;
    Serial.println("optical connected");
    break;
  }
  cnt++;
  if(cnt % 10 == 0){
    //cnt = 0;
    Serial.print("Id = : ");
    Serial.println(PId1 ,DEC);
  }
  if(cnt == 100)
    break;
  delay(1);
 }

 
 Serial.println("\n break while!! \n");
 Serial.println(cnt, DEC);
  
  OptiWriteRegister(0x89, 0xA5);
  OptiWriteRegister(0x89, 0x00);
  OptiWriteRegister(0x86, 0x00);
  OptiWriteRegister(0x86, 0x04);
  OptiWriteRegister(0x85, 0xBC);
  delay(5);
  OptiWriteRegister(0x86, 0x08);
  delay(550);
  OptiWriteRegister(0x86, 0x80);
  delay(8);
  OptiWriteRegister(0x85, 0xB9);
  OptiWriteRegister(0x86, 0x80);
  OptiWriteRegister(0x86, 0x06);
  delay(32);
}

void OptiReconnect(){
  int PId1, cnt = 0;
  while(1){
    OptiBegin();
    PId1 = OptiProductId1();
    if( PId1 == correctPId1){
      break;
    }
    cnt++;
    if(cnt % 10 == 0){
     //cnt = 0;
      Serial.print("Id = : ");
      Serial.println(PId1 ,DEC);
    }
    if(cnt == 10)
    {
      isOptiConnected = 0;
      break;
    }
    delay(1);
  }  
}

void setup() {
  signed int i;
  Serial.begin(115200);
  delay(10);

  pinMode(LEDPin, OUTPUT);

  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);

  digitalWrite(LEDPin, HIGH);
  
//  noInterrupts();
  
  Serial.println("");
  Serial.println("Start");

  OptiSetup();

}

void loop() {  // FIXME wdtが作動しないように

    Serial.print("id1=");
    Serial.print(OptiProductId1(), DEC);
    if(OptiProductId1()==48){
      pinMode(8, OUTPUT);
      digitalWrite(8, HIGH);
    }
    Serial.print(" motion=");
    Serial.print(OptiIsMotion(), DEC);
    Serial.print(" x=");
    Serial.print(OptiDx(), DEC);
    Serial.print(" y=");
    Serial.print(OptiDy(), DEC);
    
    Serial.println(); // for \n
    delay(8);
}
