// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEHIDPeripheral.h>
#include <BLEMouse.h>

#define BUTTON_LEFT_PIN 29
#define BUTTON_RIGHT_PIN 25
#define BUTTON_MID_PIN 26
#define BUTTON_FOR_PIN 8
#define BUTTON_BACK_PIN 9

// define pins (varies per shield/board)
#define BLE_REQ   9
#define BLE_RDY   8
#define BLE_RST   4

// create peripheral instance, see pinouts above
BLEHIDPeripheral bleHIDPeripheral = BLEHIDPeripheral();
BLEMouse bleMouse;

int buttonLeftState = 0;
int buttonRightState = 0;
int buttonMidState = 0;
int buttonForState = 0;
int buttonBackState = 0;

// scroll pin
byte rollUp = 28;
byte rollDown = 27;

byte lastState = 0;
byte steps = 0;
int  cw = 0;
byte rollUpState = 0;
byte rollDownState = 0;
byte State = 0;

//optical
//const int LEDPin = 14;
//const int OptiMotionPin = 14;
const int OptiDataPin = 13;
const int OptiClkPin = 12;

const unsigned char LedOnStatus = 0x11;

//int correctPId1 = 0x31;
int correctPId1 = 48;
int cnt1 = 0;

const uint8_t OptiRegDx = 0x03; // 0x03
const uint8_t OptiRegDy = 0x04; // 0x04
const uint8_t OptiRegMotion = 0x02;
const uint8_t OptiRegProd1 = 0x00;

int32_t optiX;
int32_t optiY;
signed char optiProd1;
signed char optiMotion;

char isOptiConnected = 0;


void setup() {
  Serial.begin(9600);
#if defined (__AVR_ATmega32U4__)
  while (!Serial);
#endif

  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  pinMode(BUTTON_MID_PIN, INPUT);
  pinMode(BUTTON_FOR_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);

  pinMode(rollUp, INPUT);
  pinMode(rollDown, INPUT);

  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);

  buttonLeftState = digitalRead(BUTTON_LEFT_PIN);
  buttonRightState = digitalRead(BUTTON_RIGHT_PIN);
  buttonMidState = digitalRead(BUTTON_MID_PIN);
  buttonForState = digitalRead(BUTTON_FOR_PIN);
  buttonBackState = digitalRead(BUTTON_BACK_PIN);

  delay(10);
  OptiSetup();

  //  if (buttonLeftState == LOW) {
  //    Serial.println(F("BLE HID Peripheral - clearing bond data"));
  //
  //    // clear bond store data
  //    bleHIDPeripheral.clearBondStoreData();
  //  }

  bleHIDPeripheral.clearBondStoreData();
  bleHIDPeripheral.addHID(bleMouse);
  bleHIDPeripheral.setDeviceName("OptiMice");
  bleHIDPeripheral.setLocalName("HID Mouse");

  bleHIDPeripheral.begin();



  Serial.println(F("BLE HID Mouse"));

  //  joystickXCenter = readJoystickAxis(JOYSTICK_X_AXIS_PIN);
  //  joystickYCenter = readJoystickAxis(JOYSTICK_Y_AXIS_PIN);
}

void loop() {
  BLECentral central = bleHIDPeripheral.central();

  if (central) {
    // central connected to peripheral
    Serial.print(F("Connected to central: "));
    Serial.println(central.address());

    while (central.connected()) {
      if (OptiProductId1() == 48) {
        pinMode(10, OUTPUT);
        digitalWrite(10, HIGH);
      }

      Serial.print("id1=");
      Serial.print(OptiProductId1(), DEC);
      Serial.print(" motion=");
      Serial.print(OptiIsMotion(), DEC);
      Serial.print(" x=");
      Serial.print(OptiDx(), DEC);
      Serial.print(" y=");
      Serial.print(OptiDy(), DEC);
      Serial.println(); // for \n
      bleMouse.move(OptiDx(), OptiDy() * -1);
      //      bleMouse.move(1, 1);
//      delay(1);

      //      int tempButtonState = digitalRead(JOYSTICK_BUTTON_PIN);
      int tempButtonLeftState = digitalRead(BUTTON_LEFT_PIN);
      if (tempButtonLeftState != buttonLeftState) {
        buttonLeftState = tempButtonLeftState;
        if (buttonLeftState == LOW) {
//          Serial.println(F("Left Mouse press"));
          bleMouse.press();
//          pinMode(10, OUTPUT);
//          digitalWrite(10, HIGH);
        } else {
//          Serial.println(F("Left Mouse release"));
          bleMouse.release();
//          pinMode(10, OUTPUT);
//          digitalWrite(10, LOW);
        }
      }

      int tempButtonRightState = digitalRead(BUTTON_RIGHT_PIN);
      //      if (tempButtonState != buttonState) {
      if (tempButtonRightState != buttonRightState) {
        //        buttonState = tempButtonState;
        buttonRightState = tempButtonRightState;
        if (buttonRightState == LOW) {
//          Serial.println(F("right Mouse press"));
          bleMouse.press(MOUSEBTN_RIGHT_MASK);
//          pinMode(10, OUTPUT);
//          digitalWrite(10, HIGH);
        } else {
//          Serial.println(F("right Mouse release"));
          bleMouse.release(MOUSEBTN_RIGHT_MASK);
//          pinMode(10, OUTPUT);
//          digitalWrite(10, LOW);
        }
      }

      int tempButtonMidState = digitalRead(BUTTON_MID_PIN);
      //      if (tempButtonState != buttonState) {
      if (tempButtonMidState != buttonMidState) {
        //        buttonState = tempButtonState;
        buttonMidState = tempButtonMidState;
        if (buttonMidState == LOW) {
//          Serial.println(F("Mid Mouse press"));
          bleMouse.press(MOUSEBTN_MIDDLE_MASK);
        } else {
//          Serial.println(F("Left Mouse release"));
          bleMouse.release(MOUSEBTN_MIDDLE_MASK);
        }
      }

      rollUpState = digitalRead(rollUp);
      rollDownState = digitalRead(rollDown) << 1;
      State = rollUpState | rollDownState;

      if (lastState != State) {
        switch (State) {
          case 0:
            if (lastState == 2) {
              steps++;
              cw = 1;
            }
            else if (lastState == 1) {
              steps--;
              cw = -1;
            }
            break;
          case 1:
            if (lastState == 0) {
              steps++;
              cw = 1;
            }
            else if (lastState == 3) {
              steps--;
              cw = -1;
            }
            break;
          case 2:
            if (lastState == 3) {
              steps++;
              cw = 1;
            }
            else if (lastState == 0) {
              steps--;
              cw = -1;
            }
            break;
          case 3:
            if (lastState == 1) {
              steps++;
              cw = 1;
            }
            else if (lastState == 2) {
              steps--;
              cw = -1;
            }
            break;
        }
      }

      if (State != lastState) {
        int x = 0;
        int y = 0;
        //      int x = readJoystickAxis(JOYSTICK_X_AXIS_PIN) - joystickXCenter;
        //      int y = readJoystickAxis(JOYSTICK_Y_AXIS_PIN) - joystickYCenter;


        bleMouse.move(x, y, cw);

        lastState = State;
        Serial.print(State);
        Serial.print("\t");
        Serial.print(cw);
        Serial.print("\t");
        Serial.println(steps);
        delay(1);
      }
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}


uint8_t OptiReadRegister(uint8_t address)
{
  int i = 7;
  uint8_t r = 0;
  // Write the address of the register we want to read:
  pinMode (OptiDataPin, OUTPUT);
  for (i = 7; i >= 0; i--)
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
  for (i = 7; i >= 0; i--)
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
  for (i = 7; i >= 0; i--)
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
  for (i = 7; i >= 0; i--)
  {
    digitalWrite (OptiClkPin, LOW);
    //delayMicroseconds(5);
    digitalWrite (OptiDataPin, data & (1 << i));
    delayMicroseconds(1);
    digitalWrite (OptiClkPin, HIGH);
    delayMicroseconds(5);
  }
}

signed char OptiDx() {
  return  (signed char) OptiReadRegister(OptiRegDx);
}

signed char OptiDy() {
  return  (signed char) OptiReadRegister(OptiRegDy);
}

signed char OptiProductId1() {
  return  (signed char) OptiReadRegister(OptiRegProd1);
}

signed char OptiIsMotion() {
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

void OptiSetup() {
  int cnt = 0, PId1;
  pinMode (OptiClkPin, OUTPUT);
  pinMode (OptiDataPin, INPUT);
  Serial.println("OptiSetup");
  while (1) {
    OptiBegin();
    PId1 = OptiProductId1();
    if ( PId1 == correctPId1) {
      isOptiConnected = 1;
      break;
    }
    cnt++;
    if (cnt % 10 == 0) {
      //cnt = 0;
      Serial.print("Id = : ");
      Serial.println(PId1 , DEC);
    }
    if (cnt == 100)
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

void OptiReconnect() {
  int PId1, cnt = 0;
  while (1) {
    OptiBegin();
    PId1 = OptiProductId1();
    if ( PId1 == correctPId1) {
      break;
    }
    cnt++;
    if (cnt % 10 == 0) {
      //cnt = 0;
      Serial.print("Id = : ");
      Serial.println(PId1 , DEC);
    }
    if (cnt == 10)
    {
      isOptiConnected = 0;
      break;
    }
    delay(1);
  }
}
