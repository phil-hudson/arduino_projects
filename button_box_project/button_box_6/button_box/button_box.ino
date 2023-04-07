#include <Wire.h>
#include <Joystick.h>


//JOYSTICK SETTINGS
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK,
                   40, //number of buttons
                   0, //number of hat switches
                   //Set as many axis to "true" as you have potentiometers for
                   false, // x axis
                   false, // y axis
                   false, // z axis
                   false, // rx axis
                   false, // ry axis
                   false, // rz axis
                   false, // rudder
                   false, // throttle
                   false, // accelerator
                   false, // brake
                   false); // steering wheel



static int totalPins = 13;
int oldPinValues[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
int pinValues[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
int pinsInUse[13] = {0,0,1,1,1,1,1,1,1,1,1,1,1};

void setup() {
pinMode (3, INPUT);
pinMode (4, INPUT);
pinMode (5, INPUT);
pinMode (6, INPUT);
pinMode (7, INPUT);
pinMode (8, INPUT);
pinMode (9, INPUT);
pinMode (10, INPUT);
pinMode (11, INPUT);
pinMode (12, INPUT);

// todo off by 1
//  for (int i = 1; i < 12; i++) {
//    if (pinsInUse[i] == 1) {
//            pinMode (i, INPUT);
//
//    }
//
//  }

  Serial.println("Start setup");

  Joystick.begin();

  Serial.begin(9600);
//  while (! Serial) {
//    //wait for Serial connection
//  }

}

void loop() {
  handleControls();
}

void readPinVals() {
  for (int i = 1; i < totalPins; i++) {
    if (pinsInUse[i] != 1) {
      continue;
    }
    if (digitalRead(i) == LOW) {
      pinValues[i] = 0;
    }
    else {
      pinValues[i] = 1;
    }
  }
}

void display_pin_values() {
      Serial.print("*Pin value change detected*\r\n");

  for (int i = 1; i < totalPins; i++) {
       char buffer[64];  // buffer must be big enough to hold all the message
   sprintf(buffer, "index: %d old: %d New: %d", i, oldPinValues[i], pinValues[i]);
   Serial.println(buffer);
  }
}

boolean pins_changed() {
    for (int i = 1; i < totalPins; i++) {
     if (pinValues[i] != oldPinValues[i]) {
      return true;
     }
  }
  return false;
}

void handleControls() {
  /* Read the state of all zones.
  */
  readPinVals();

  /* If there was a chage in state, display which ones changed.
  */
  if (pins_changed())
  {
    display_pin_values();
    dispatchChangedPinStates();
    memcpy(oldPinValues, pinValues, sizeof(pinValues));
  }

  delay(25);
}

void dispatchChangedPinStates() {
  for (int i = 1; i < totalPins; i++)
  {
    if (oldPinValues[i] != pinValues[i]) {
      handlePinStateChange(i, pinValues[i] == 1 ? true: false);
    }

  }
}


void handlePinStateChange(int pin, bool state) {
  Joystick.setButton(pin, state);
}
