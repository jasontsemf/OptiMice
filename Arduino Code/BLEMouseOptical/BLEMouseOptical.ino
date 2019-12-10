#include <SPI.h>
#include <BLEPeripheral.h>
#include <BLEHIDPeripheral.h>
#include <BLEMouse.h>
#include <BLEKeyboard.h>
#include <BLEMultimedia.h>
#include <BLESystemControl.h>

//BLEHIDPeripheral bleHIDPeripheral = BLEHIDPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEHIDPeripheral bleHIDPeripheral = BLEHIDPeripheral();
BLEMouse bleMouse;
BLEKeyboard bleKeyboard;
BLEMultimedia bleMultimedia;
BLESystemControl bleSystemControl;

#define LMB 23
#define RMB 24
#define MMB 25
#define FB 35
#define BB 36
#define SCRLUP 22
#define SCRLDN 21
//#define SCRLUP 6
//#define SCRLDN 7

// optical mouse
const int OptiDataPin = 2;
const int OptiClkPin = 1;

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

int16_t optiSpX;
int16_t optiSpY;

const unsigned int optiSpCof = 800; // 1000分率
const int optiSpDif = 1000;

char isOptiConnected = 0;

// mouse button state
int leftState;
int rightState;
int middleState;

// side buttons state
int forwardState;
int backwardState;

// wheel state
byte rollUp = 6;
byte rollDown = 7;

byte lastState = 0;
byte steps = 0;
int  cw = 0;
int mStep = 0;
byte rollUpState = 0;
byte rollDownState = 0;
byte State = 0;

void setup() {
  delay(100);
  Serial.begin(115200);
  
#if defined (__AVR_ATmega32U4__)
  while(!Serial);
  delay(5000);
#endif

//setup optical
  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);
  
  noInterrupts();
  OptiSetup();
  interrupts();
  
//setup buttons
//  pinMode(PIN_BUTTON_A, INPUT);
//  pinMode(PIN_BUTTON_B, INPUT);
  pinMode(LMB, INPUT);
  pinMode(RMB, INPUT);
  pinMode(MMB, INPUT);
//  leftState = digitalRead(PIN_BUTTON_A);
//  rightState = digitalRead(PIN_BUTTON_B);
  leftState = digitalRead(LMB);
  rightState = digitalRead(RMB);
  middleState = digitalRead(MMB);

//setup side buttons
  pinMode(FB, INPUT);
  pinMode(BB, INPUT);
  forwardState = digitalRead(FB);
  backwardState = digitalRead(BB);
  
//setup wheel
  pinMode(SCRLUP, INPUT);
  pinMode(SCRLDN, INPUT);

  bleHIDPeripheral.clearBondStoreData();
  bleHIDPeripheral.setLocalName("OptiMice");
  bleHIDPeripheral.setDeviceName("OptiMice");
  
//   must be in this order bleKeyboard > bleMouse > bleMultimedia
  bleHIDPeripheral.addHID(bleSystemControl);
  bleHIDPeripheral.addHID(bleMultimedia);
  bleHIDPeripheral.addHID(bleMouse);
  bleHIDPeripheral.addHID(bleKeyboard);

  bleHIDPeripheral.begin();
  Serial.println(F("BLE HID Mouse"));
  
}

void loop() {
  BLECentral central = bleHIDPeripheral.central();

  if (central) {
    // central connected to peripheral
    Serial.print(F("Connected to central: "));
    Serial.println(central.address());

    while (central.connected()) {
      
//        Serial.print("id1=");
//        Serial.print(OptiProductId1(), DEC);
//        signed char x = OptiDx();
//        signed char y = OptiDy();
        Serial.print(" motion=");
        Serial.println(OptiIsMotion(), DEC);
//        Serial.print(" x=");
//        Serial.print(OptiDx(), DEC);
//        Serial.print(" y=");
//        Serial.println(OptiDy(), DEC);

      bleMouse.move(OptiDx(), OptiDy()*-1);
//      int tempLeftState = digitalRead(PIN_BUTTON_A);
      int tempLeftState = digitalRead(LMB);
      if (tempLeftState != leftState) {
        leftState = tempLeftState;
        if (leftState == LOW) {
          Serial.println(F("Left Mouse press"));
          bleMouse.press();
//          bleMultimedia.write(MMKEY_MUTE);
        } else {
          Serial.println(F("Left Mouse release"));
          bleMouse.release();
        }
      }
      
//      int tempRightState = digitalRead(PIN_BUTTON_B);
      int tempRightState = digitalRead(RMB);
      if (tempRightState != rightState) {
        rightState = tempRightState;
        if (rightState == LOW) {
          Serial.println(F("Right Mouse press"));
          bleMouse.press(MOUSEBTN_RIGHT_MASK);
        } else {
          Serial.println(F("Right Mouse release"));
          bleMouse.release(MOUSEBTN_RIGHT_MASK);
        }
      }
      
      int tempMiddleState = digitalRead(MMB);
      if (tempMiddleState != middleState) {
        middleState = tempMiddleState;
        if (middleState == LOW) {
          Serial.println(F("Middle Mouse press"));
          bleMouse.press(MOUSEBTN_MIDDLE_MASK);
        } else {
          Serial.println(F("Middle Mouse release"));
          bleMouse.release(MOUSEBTN_MIDDLE_MASK);
        }
      }   

      int tempForwardState = digitalRead(FB);
      if (tempForwardState != forwardState) {
        forwardState = tempForwardState;
        if (forwardState == LOW) {
          Serial.println(F("Forward Mouse press"));
          bleKeyboard.press(KEYCODE_ARROW_LEFT,KEYCODE_MOD_LEFT_GUI);
        } else {
          Serial.println(F("Forward Mouse release"));
          bleKeyboard.release(KEYCODE_ARROW_LEFT,KEYCODE_MOD_LEFT_GUI);
        }
      } 

      int tempBackwardState = digitalRead(BB);
      if (tempBackwardState != backwardState) {
        backwardState = tempBackwardState;
        if (backwardState == LOW) {
          Serial.println(F("Backward Mouse press"));
          bleKeyboard.press(KEYCODE_ARROW_RIGHT,KEYCODE_MOD_LEFT_GUI);
        } else {
          Serial.println(F("Backward Mouse release"));
          bleKeyboard.release(KEYCODE_ARROW_RIGHT,KEYCODE_MOD_LEFT_GUI);
        }
      } 

      //mouse move and scroll
      updateWheel();
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
  delay(4);
}

void updateWheel(){
  rollUpState = digitalRead(rollUp);
  rollDownState = digitalRead(rollDown) << 1;
  State = rollUpState | rollDownState;

  if (lastState != State){
    switch (State) {
      case 0:
        if (lastState == 2){
          steps++;
          cw = 1;
        }
        else if(lastState == 1){
          steps--;
          cw = -1;
        }
        break;
      case 1:
        if (lastState == 0){
          steps++;
          cw = 1;
        }
        else if(lastState == 3){
          steps--;
          cw = -1;
        }
        break;
      case 2:
        if (lastState == 3){
          steps++;
          cw = 1;
        }
        else if(lastState == 0){
          steps--;
          cw = -1;
        }
        break;
      case 3:
        if (lastState == 1){
          steps++;
          cw = 1;
        }
        else if(lastState == 2){
          steps--;
          cw = -1;
        }
        break;
    }
  }

  if(lastState != State && State != 2){
    bleMouse.move(0, 0, cw);
    Serial.print("scroll");
    Serial.print("\t");
    Serial.println("cw");
  }
  lastState = State;
  delay(1);
}

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
  if( PId1 == correctPId1){
    isOptiConnected = 1;
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
