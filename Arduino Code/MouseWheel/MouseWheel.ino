byte rollUp = 6;
byte rollDown = 7;

byte lastState = 0;
byte steps = 0;
int  cw = 0;
byte rollUpState = 0;
byte rollDownState = 0;
byte State = 0;

void setup() {
  Serial.begin(9600);
  pinMode(rollUp, INPUT);
  pinMode(rollDown, INPUT);
}

void loop() {
  // read the input pin:
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

  lastState = State;
  Serial.print(State);
  Serial.print("\t");
  Serial.print(cw);
  Serial.print("\t");
  Serial.println(steps);
  delay(1);
}
