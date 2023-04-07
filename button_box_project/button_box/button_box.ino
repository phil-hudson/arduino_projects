//partially based off of: deej_Controller_No_EEPROM from https://github.com/YaMoef/deej
#include <Wire.h>
#include  <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <Joystick.h>

/*
   SN74HC165N_shift_reg

   Program to shift in the bit values from a SN74HC165N 8-bit
   parallel-in/serial-out shift register.

   This sketch demonstrates reading in 16 digital states from a
   pair of daisy-chained SN74HC165N shift registers while using
   only 4 digital pins on the Arduino.

   You can daisy-chain these chips by connecting the serial-out
   (Q7 pin) on one shift register to the serial-in (Ds pin) of
   the other.

   Of course you can daisy chain as many as you like while still
   using only 4 Arduino pins (though you would have to process
   them 4 at a time into separate unsigned long variables).

*/


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

// from https://playground.arduino.cc/Code/ShiftRegSN74HC165N/

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   5 // for prod change to 5

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

/* You will need to change the "int" to "long" If the
   NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

int ploadPin        = 9;  // Connects to Parallel load pin the 165
int clockEnablePin  = 10;  // Connects to Clock Enable pin the 165
int dataPin         = 11; // Connects to the Q7 pin the 165
int clockPin        = 8; // Connects to the Clock pin the 165

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

/* This function is essentially a "shift-in" routine reading the
   serial Data from the shift register chips and representing
   the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
  long bitVal;
  BYTES_VAL_T bytesVal = 0;

  /* Trigger a parallel Load to latch the state of the data lines,
  */
  digitalWrite(clockEnablePin, HIGH);
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);
  digitalWrite(clockEnablePin, LOW);

  /* Loop to read each bit value from the serial out line
     of the SN74HC165N.
  */
  for (int i = 0; i < DATA_WIDTH; i++)
  {
    bitVal = digitalRead(dataPin);

    /* Set the corresponding bit in bytesVal.
    */
    bytesVal |= (bitVal << ((DATA_WIDTH - 1) - i));

    /* Pulse the Clock (rising edge shifts the next bit).
    */
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(clockPin, LOW);
  }

  return (bytesVal);
}

/* Dump the list of zones along with their current status.
*/
void display_pin_values()
{
  Serial.print("Pin States:\r\n");

  for (int i = 0; i < DATA_WIDTH; i++)
  {
    if (i % 8 == 0) {
      Serial.print("  Chip- ");
      Serial.print((i/8) + 1);
        Serial.print("\r\n");


    }
    Serial.print("  Pin-");
    Serial.print(i);
    Serial.print(": ");

    BYTES_VAL_T newShift = pinValues >> i;
    BYTES_VAL_T oldShift = oldPinValues >> i;
    int newShifted = newShift & 1; //get 0th index bit
    int oldShifted = oldShift & 1; //get 0th index bit
    if (newShifted)
      Serial.print("HIGH");
    else
      Serial.print("LOW");

    if (newShifted != oldShifted) {
      Serial.print(" << CHANGED");
    }

    Serial.print("\r\n");
  }

  Serial.print("\r\n");
}

void setupShiftReg() {
  /* Initialize our digital pins...
  */
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);

  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);

  /* Read in and display the pin states at startup.
  */
  pinValues = read_shift_regs();
  display_pin_values();
  oldPinValues = pinValues;
  Serial.println("Finshed shift reg setup");
}

// END SHIFT REG

// Wiring: SDA pin is connected to D2 and SCL pin to D3.
// Connect to LCD via I2C, default address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define TIME_HEADER  'T'   // Header tag for serial time sync message


//you can tweak following values for you needs
const int encoder0PinA = 7; //encoder pins
const int encoder0PinB = 5; //encoder pins
const int buttonpin = 6;//pushbutton from encoder
const int amountsliders = 4; //amount of sliders you want, also name them in the array below
const String slidernames[amountsliders] = {"Master",
                                           "iRacing",
                                           "Discord",
                                           "Browser",
                                          };
const int increment[amountsliders] = {1, 1, 1, 1}; //choose you're increment for each slider 1,2,4,5,10,20,25,50,100

int sliderHadAction[amountsliders] = {0, 0, 0, 0}; // binary array to indicate slider movement 1 == increase -1 == decrease

//leave following values at their default
bool encoder0PinALast = HIGH;//start values for the encoder
bool firstEncoderRead = LOW;

bool lcdupdate = 1; //when this variable is 1 the lcd will update and the variable will turn back to 0
bool sliderupdate = 1; //when this variable is 1 the slidervalues get sent to deej and the variable will turn back to 0
int slidernumber = 0; //variable which numbers all the sliders
bool singlebuttonpress = 0; //variable to let the pushbutton from the encoder toggle 1

int state = 0;    //state 0 is the menu screen to select what you want to change
//state 1 is the screen where you change the value itself

// clock state
static const unsigned long TIME_REFRESH_INTERVAL = 60000; // ms
static unsigned long lastTimeRefreshTime = 0;
boolean clockEverDisplayed = false;

byte arrow[] = {  //byte for creating an arrow on the lcd screen
  B11000,
  B11100,
  B11110,
  B11111,
  B11110,
  B11100,
  B11000,
  B00000
};

void setupLCD() {
  //  lcd.begin(16,2);                      // initialize the lcd
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.print("SETUP");
  awaitingClient();
  clockNotSynced();
  Serial.println("Finshed LCD setup");
}


void setup() {
  // volume button encoder
  pinMode (encoder0PinA, INPUT); // change to input when using encoder
  pinMode (encoder0PinB, INPUT); // change to input when using encoder
  pinMode (buttonpin, INPUT_PULLUP);

  Serial.println("Start setup");

  Joystick.begin();

  setupShiftReg();
  setupLCD();

  Serial.begin(9600);
  while (! Serial) {
    //wait for Serial connection
  }

}

void loop() {
  handleControls();
  handleAudioManagement();

  if (timeStatus() == timeNotSet && Serial.available()) {
    processTimeSyncMessage();
  }

  handleClock();
}


void handleControls() {
  /* Read the state of all zones.
  */
  pinValues = read_shift_regs();

  /* If there was a chage in state, display which ones changed.
  */
  if (pinValues != oldPinValues)
  {
    Serial.print("*Pin value change detected*\r\n");
    display_pin_values();
    dispatchChangedPinStates(pinValues);
    oldPinValues = pinValues;
  }

  delay(POLL_DELAY_MSEC);
}

void dispatchChangedPinStates(BYTES_VAL_T pinVals) {
  for (int i = 0; i < DATA_WIDTH; i++)
  {

    BYTES_VAL_T newShift = pinVals >> i;
    BYTES_VAL_T oldShift = oldPinValues >> i;
    int newShifted = newShift & 1; //get 0th index bit
    int oldShifted = oldShift & 1; //get 0th index bit


    if (newShifted != oldShifted) {
      handlePinStateChange(i, newShifted);
    }

  }
}

void handlePinStateChange(int pin, bool state) {
  Joystick.setButton(pin, state);
}

void processTimeSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}

void handleClock() {

  if (clockEverDisplayed == false || millis() - lastTimeRefreshTime >= TIME_REFRESH_INTERVAL)
  {
    lastTimeRefreshTime = millis();
    if (timeStatus() != timeNotSet) {
      digitalClockDisplay();
    } else {
      clockNotSynced();
    }
  }
}

void digitalClockDisplay() {
  // digital clock display of the time
  clockEverDisplayed = true;
  clearLCDLine(1);
  lcd.setCursor(0, 1); // set to second row
  //  String msg = getTimeIntToString(hour()) + ":" +  getTimeIntToString(minute()) + ":" +  getTimeIntToString(second());
  String msg = getTimeIntToString(hour()) + ":" +  getTimeIntToString(minute());
  lcd.print(msg);
}

void clockNotSynced() {
  lcd.setCursor(0, 1); // set to second row
  lcd.print("Clock not synced");
}

void awaitingClient() {
  lcd.setCursor(0, 0); // set to second row
  lcd.print("Awaiting client");
}

void handleAudioManagement() {
  if ((digitalRead(buttonpin) == LOW) && singlebuttonpress == 0) { //this reads the button of the encoder and will only react to the first press
    Serial.println("press");
    singlebuttonpress = 1;
    lcdupdate = 1;
    if (state == 0) {
      state = 1;
    }
    else if (state == 1) {
      state = 0;
      slidernumber = 0;
    }
    delay(10);
  }
  if (digitalRead(buttonpin) == HIGH) {
    singlebuttonpress = 0;
  }

  firstEncoderRead = digitalRead(encoder0PinA);//determing if encoder is turning, and if so, what side it is turning
  if ((encoder0PinALast == LOW) && (firstEncoderRead == HIGH)) {
    lcdupdate = 1;
    if (digitalRead(encoder0PinB) == LOW) {
      if (state == 0) { //scrolling between all the slides
        slidernumber++;
        if (slidernumber > (amountsliders - 1)) {
          slidernumber = 0;
        }
      }
      else if (state == 1) {
        sliderHadAction[slidernumber] = 1;
        sliderupdate = 1;
      }
    }
    else {
      if (state == 0) {
        slidernumber--;
        if (slidernumber < 0) {
          slidernumber = amountsliders - 1;
          sliderupdate = 1;
        }
      }
      else if (state == 1) {
        sliderHadAction[slidernumber] = -1;
        sliderupdate = 1;
      }
    }
  }
  encoder0PinALast = firstEncoderRead;


  handleAudioManagementLCDUpdates();
  handleAudioManagementSliderUpdates();
}

void handleAudioManagementLCDUpdates() {
  if (lcdupdate == 1) { //update the lcd with new values
    lcdupdate = 0;
    clearLCDLine(0);
    // state 0 no slider selected
    if (state == 0) {
      lcd.setCursor(0, 0); //put slider names on screen
      lcd.print(slidernames[slidernumber]);
    }
    // state 1: slider selected
    else if (state == 1) { //put slider on screen
      lcd.setCursor(0, 0);
      lcd.print(slidernames[slidernumber] + " +/-");
    }
  }
}

void handleAudioManagementSliderUpdates() {
  if (sliderupdate == 1) { //update the slider values
    sliderupdate == 0;
    String builtString = buildString();
    // no need to update if no change
    if (builtString != "0|0|0|0") {
      Serial.println(builtString);//combining every slider values seperated by | and sending it through the serial console
    }
  }
}

//TODO fix the bug with a missing seperator or something
String buildString() {
  String builtString = String("");
  for (int index = 0; index < amountsliders; index++) {
    if (sliderHadAction[index] == 0) {
      builtString += String(0);
    } else if (sliderHadAction[index] == 1) {
      builtString += String(increment[index]);

    } else if (sliderHadAction[index] == -1) {
      builtString += String(increment[index] * -1);

    }
    if (index < amountsliders - 1) {
      builtString += String("|");
    }
    // reset slider action
    sliderHadAction[index] = 0;
  }
  return builtString;
}

void clearLCDLine(int line)
{
  lcd.setCursor(0, line);
  for (int n = 0; n < 16; n++) // 16 indicates symbols in line. For 2x16 LCD write - 16
  {
    lcd.print(" ");
  }
}

String getTimeIntToString(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  String result = "";
  if (digits < 10) {
    result += "0";
  }
  result +=  String(digits);
  return result;
}
