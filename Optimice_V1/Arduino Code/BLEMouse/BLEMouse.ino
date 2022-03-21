#include <SPI.h>
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

//#define LMB 1
//#define RMB 2
//#define MMB 3
//#define FB 4
//#define BB 5
//#define SCRLUP 6
//#define SCRLDN 7

// optical mouse
//const int LEDPin = 14; 
const int OptiDataPin = 11;
const int OptiClkPin = 12;

int correctPId1 = 33;
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
#endif

//setup optical
//  delay(10);
  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);
  
//setup buttons
  pinMode(PIN_BUTTON_A, INPUT);
  pinMode(PIN_BUTTON_B, INPUT);
//  pinMode(MMB, INPUT);
  leftState = digitalRead(PIN_BUTTON_A);
  rightState = digitalRead(PIN_BUTTON_B);
//  middleState = digitalRead(MMB);

//setup side buttons
//  pinMode(FB, INPUT);
//  pinMode(BB, INPUT);
//  forwardState = digitalRead(FB);
//  backwardState = digitalRead(BB);
  
//setup wheel
//  pinMode(SCRLUP, INPUT);
//  pinMode(SCRLDN, INPUT);

  bleHIDPeripheral.clearBondStoreData();
  bleHIDPeripheral.setLocalName("OptiMice");
  bleHIDPeripheral.setDeviceName("OptiMice");

  // must be in this order bleKeyboard > bleMouse > bleMultimedia
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
      int tempLeftState = digitalRead(PIN_BUTTON_A);
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
      
      int tempRightState = digitalRead(PIN_BUTTON_B);
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
//      
//      int tempMiddleState = digitalRead(MMB);
//      if (tempMiddleState != middleState) {
//        rightState = tempRightState;
//        if (middleState == LOW) {
//          Serial.println(F("Middle Mouse press"));
//          bleMouse.press(MOUSEBTN_MIDDLE_MASK);
//        } else {
//          Serial.println(F("Middle Mouse release"));
//          bleMouse.release(MOUSEBTN_MIDDLE_MASK);
//        }
//      }   
//
//      int tempForwardState = digitalRead(FB);
//      if (tempForwardState != forwardState) {
//        forwardState = tempForwardState;
//        if (forwardState == LOW) {
//          Serial.println(F("Forward Mouse press"));
//          bleKeyboard.press(KEYCODE_ARROW_LEFT,KEYCODE_MOD_LEFT_GUI);
//        } else {
//          Serial.println(F("Forward Mouse release"));
//          bleKeyboard.release(KEYCODE_ARROW_LEFT,KEYCODE_MOD_LEFT_GUI);
//        }
//      } 
//
//      int tempBackwardState = digitalRead(BB);
//      if (tempBackwardState != backwardState) {
//        backwardState = tempBackwardState;
//        if (backwardState == LOW) {
//          Serial.println(F("Backward Mouse press"));
//          bleKeyboard.press(KEYCODE_ARROW_RIGHT,KEYCODE_MOD_LEFT_GUI);
//        } else {
//          Serial.println(F("Backward Mouse release"));
//          bleKeyboard.release(KEYCODE_ARROW_RIGHT,KEYCODE_MOD_LEFT_GUI);
//        }
//      } 

      //mouse move and scroll
//      bleMouse.move(OptiDx(), OptiDy());
//      updateWheel();
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
    
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
//    Serial.print("scroll");
//    Serial.print("\t");
  }
//  lastState = State;
  delay(1);
}
