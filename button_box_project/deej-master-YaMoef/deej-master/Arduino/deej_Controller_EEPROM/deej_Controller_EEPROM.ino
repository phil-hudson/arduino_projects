#include <LiquidCrystal_I2C.h>//library for an i2C display, change this to an regular lcd library to use an normal lcd
#include <EEPROM.h>//library to use the eeprom

LiquidCrystal_I2C lcd(0x27, 16, 2); //don't forget to change this line when using an regular lcd
//if the I2C lcd is not working, check you connection and the address above (0x27);

//you can tweak following values for you needs
const int encoder0PinA = 6;//encoder pins if you want the encoder to invert, just switch the to pins
const int encoder0PinB = 7;
const int buttonpin = 8;//pushbutton from encoder
const int amountsliders = 7;//amount of sliders you want, also name them in the array below
const String slidernames[amountsliders] = {"Master",
                                           "Games",
                                           "Browser",
                                           "Music",
                                           "Communication",
                                           "Mic",
                                           "Teams",
                                          };
const int increment[amountsliders] = {5, 2, 1, 5, 5, 5, 2}; //choose you're increment for each slider 1,2,4,5,10,20,25,50,100
int displayValue[amountsliders] = {100, 100, 100, 100, 100, 100, 100}; //start values for every slider
int previousValue[amountsliders] = {100, 100, 100, 100, 100, 100, 100}; //extra values to see if it changed compared to last cycle

//leave following values at their default
bool encoder0PinALast = HIGH;//start values for the encoder
bool firstEncoderRead = LOW;
bool lcdupdate = 1;  //when this variable is 1 the lcd will update and the variable will turn back to 0
bool sliderupdate = 1; //when this variable is 1 the slidervalues get sended to deej and the variable will turn back to 0
int slidernumber = 0; //variable which numbers all the sliders
bool singlebuttonpress = 0;//variable to let the pushbutton from the encoder toggle 1
int state = 0;      //state 0 is the menu screen to select what you want to change
//state 1 is the screen where you change the value itself
int EEPROMvalue = 0;
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

void setup() {
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  pinMode (buttonpin, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, arrow);
  lcd.home();
  Serial.begin(9600);
  while (! Serial) {
    //wait for Serial connection
  }
  for (int i = 0; i < amountsliders; i++) { //reading values out of the eeprom
    EEPROMvalue = EEPROM.read(i);
    if ((EEPROMvalue != 0) && (EEPROMvalue <= 100)) {
      displayValue[i] = EEPROMvalue;
    }
    else {
      EEPROM.write(i, 100); //if the values is not correct write 100 to it
    }
    previousValue[i] = displayValue[i];
  }
}

void loop() {
  if ((digitalRead(buttonpin) == LOW) && singlebuttonpress == 0) { //this reads the button of the encoder and will only react to the first press
    singlebuttonpress = 1;
    lcdupdate = 1;
    if (state == 0) {
      state = 1;
    }
    else if (state == 1) {
      state = 0;
      for (int i = 0; i < amountsliders; i++) { //only update when going back to main menu and the value needs to be changed
        if (previousValue[i] != displayValue[i]) {
          EEPROM.update(i, displayValue[i]);//writing value to EEPROM
        }
        previousValue[i] = displayValue[i];
      }
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
        if ((100 - displayValue[slidernumber]) >= increment[slidernumber]) { //increasing slider
          displayValue[slidernumber] = displayValue[slidernumber] + increment[slidernumber];
          sliderupdate = 1;
        }
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
        if (displayValue[slidernumber] >= increment[slidernumber]) { //decreasing slider
          displayValue[slidernumber] = displayValue[slidernumber] - increment[slidernumber];
        }
      }
    }
  }
  encoder0PinALast = firstEncoderRead;

  if (lcdupdate == 1) { //update the lcd with new values
    lcdupdate = 0;
    lcd.clear();
    if (state == 0) {
      lcd.setCursor(0, 0); //put sliders names on screen
      lcd.write(0);
      lcd.print(slidernames[slidernumber]);
      lcd.setCursor(1, 1);
      if (slidernumber >= (amountsliders - 1)) {
        lcd.print(slidernames[0]);
      }
      else {
        lcd.print(slidernames[slidernumber + 1]);
      }
    }
    else if (state == 1) { //put slider on screen
      lcd.setCursor(0, 0);
      lcd.print(slidernames[slidernumber]);
      lcd.setCursor(0, 1);
      lcd.print(displayValue[slidernumber]);
      lcd.print("%");
    }
  }
  if (sliderupdate == 1) { //update the slider values
    sliderupdate == 0;
    String builtString = String("");
    for (int index = 0; index < amountsliders; index++) {
      builtString += String(displayValue[index]);
      if (index < amountsliders - 1) {
        builtString += String("|");
      }
    }
    Serial.println(builtString);//combining every slider values seperated by | and sending it through the serial console
  }
}
