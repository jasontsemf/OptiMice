const int buttonLeftPin = 11;
const int buttonRightPin = 10;
const int buttonMidPin = 12;
const int buttonForPin = 14;
const int buttonBackPin = 13;

int buttonLeftState = 0;
int buttonRightState = 0;
int buttonMidState = 0;
int buttonForState = 0;
int buttonBackState = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonLeftPin, INPUT);
  pinMode(buttonRightPin, INPUT);
  pinMode(buttonMidPin, INPUT);
  pinMode(buttonForPin, INPUT);
  pinMode(buttonBackPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonLeftState = digitalRead(buttonLeftPin);
  buttonRightState = digitalRead(buttonRightPin);
  buttonMidState = digitalRead(buttonMidPin);
  buttonForState = digitalRead(buttonForPin);
  buttonBackState = digitalRead(buttonBackPin);
  
  if(buttonLeftState == HIGH){
    Serial.println("buttonLeftState is HIGH");
  }
  if(buttonRightState == HIGH){
    Serial.println("buttonRightState is HIGH");
  }
  if(buttonMidState == HIGH){
    Serial.println("buttonMidState is HIGH");
  }
  if(buttonForState == HIGH){
    Serial.println("buttonForState is HIGH");
  }
  if(buttonBackState == HIGH){
    Serial.println("buttonBackState is HIGH");
  }
}
