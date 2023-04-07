#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int ldrPin = A0;
int lastMessageExpire = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // initialize the serial communications:
  Serial.begin(9600);

  pinMode(ldrPin, INPUT);
}

void loop()
{
  // when characters arrive over the serial port...
  if (Serial.available()) {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) {
      // display each character to the LCD
      lcd.write(Serial.read());
    }
    lastMessageExpire = millis() + 5000;

  } else if (millis() > lastMessageExpire)  {
    // print brightness
    int ldrStatus = analogRead(ldrPin); // 0 - 1023 range, 0 darkest
    lcd.clear();
    lcd.print("Brightness: " + String(ldrStatus));
  }

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 2);
  // print the number of seconds since reset:
  lcd.print("Uptime: " + String(millis() / 1000, 10));
}
