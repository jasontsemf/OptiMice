// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEHIDPeripheral.h>
#include <BLEMouse.h>

#define BUTTON_LEFT_PIN 22
#define BUTTON_RIGHT_PIN 25
#define BUTTON_MID_PIN 24
#define BUTTON_FOR_PIN 14
#define BUTTON_BACK_PIN 13

#define JOYSTICK_X_AXIS_PIN A0
#define JOYSTICK_Y_AXIS_PIN A1
#define JOYSTICK_RANGE 24

// define pins (varies per shield/board)
#define BLE_REQ   9
#define BLE_RDY   8
#define BLE_RST   4

// create peripheral instance, see pinouts above
BLEHIDPeripheral bleHIDPeripheral = BLEHIDPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEMouse bleMouse;

int buttonLeftState = 0;
int buttonRightState = 0;
int buttonMidState = 0;
int buttonForState = 0;
int buttonBackState = 0;

int joystickXCenter;
int joystickYCenter;

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

  buttonLeftState = digitalRead(BUTTON_LEFT_PIN);
  buttonRightState = digitalRead(BUTTON_RIGHT_PIN);
  buttonMidState = digitalRead(BUTTON_MID_PIN);
  buttonForState = digitalRead(BUTTON_FOR_PIN);
  buttonBackState = digitalRead(BUTTON_BACK_PIN);

  //  if (buttonLeftState == LOW) {
  //    Serial.println(F("BLE HID Peripheral - clearing bond data"));
  //
  //    // clear bond store data
  //    bleHIDPeripheral.clearBondStoreData();
  //  }

  bleHIDPeripheral.clearBondStoreData();
  bleHIDPeripheral.setDeviceName("Arduino BLE HID jason");
  bleHIDPeripheral.setLocalName("HID Mouse");
  bleHIDPeripheral.addHID(bleMouse);

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
      //      int tempButtonState = digitalRead(JOYSTICK_BUTTON_PIN);
      int tempButtonLeftState = digitalRead(BUTTON_LEFT_PIN);
      //      if (tempButtonState != buttonState) {
      if (tempButtonLeftState != buttonLeftState) {
        //        buttonState = tempButtonState;
        buttonLeftState = tempButtonLeftState;
        if (buttonLeftState == LOW) {
          Serial.println(F("Left Mouse press"));
          bleMouse.press();
        } else {
          Serial.println(F("Left Mouse release"));
          bleMouse.release();
        }
      }

      int tempRightLeftState = digitalRead(BUTTON_RIGHT_PIN);
      //      if (tempButtonState != buttonState) {
      if (tempRightLeftState != buttonRightState) {
        //        buttonState = tempButtonState;
        buttonRightState = tempRightLeftState;
        if (buttonRightState == LOW) {
          Serial.println(F("Mid Mouse press"));
          bleMouse.press(MOUSEBTN_RIGHT_MASK);
        } else {
          Serial.println(F("Left Mouse release"));
          bleMouse.release(MOUSEBTN_RIGHT_MASK);
        }
      }

      int tempMidLeftState = digitalRead(BUTTON_MID_PIN);
      //      if (tempButtonState != buttonState) {
      if (tempMidLeftState != buttonMidState) {
        //        buttonState = tempButtonState;
        buttonMidState = tempMidLeftState;
        if (buttonMidState == LOW) {
          Serial.println(F("Mid Mouse press"));
          bleMouse.press(MOUSEBTN_MIDDLE_MASK);
        } else {
          Serial.println(F("Left Mouse release"));
          bleMouse.release(MOUSEBTN_MIDDLE_MASK);
        }
      }


      int x = 0;
      int y = 0;
      //      int x = readJoystickAxis(JOYSTICK_X_AXIS_PIN) - joystickXCenter;
      //      int y = readJoystickAxis(JOYSTICK_Y_AXIS_PIN) - joystickYCenter;

      if (x || y) {
        bleMouse.move(x, y);
      }
    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}

int readJoystickAxis(int pin) {
  int rawValue = analogRead(pin);
  int mappedValue = map(rawValue, 0, 1023, 0, JOYSTICK_RANGE);
  int centeredValue = mappedValue - (JOYSTICK_RANGE / 2);

  return (centeredValue * -1); // reverse direction
}
