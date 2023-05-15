#include <Arduino.h>
#include <HomeSpan.h>
#include <IRsend.h>
#include <IRrecv.h>  // https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/IRrecv.h
#include <EEPROM.h>
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <IRutils.h>

// CONFIGURATION PART *************************************************************************

// Set DEBUGGING for addiditonal debugging output over serial
#define DEBUGGING
#define ESP32
// set Hostname
#define HOSTNAME "HUDSON-AC-CONTROLLER"

// END CONFIG *********************************************************************************

#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

// pin layout for esp32

const uint16_t PIN_BUTTON = 14;
const uint16_t PIN_RED = 27;
const uint16_t PIN_GREEN = 26;
const uint16_t PIN_BLUE = 25;
const uint16_t PIN_IR_RECV = 15;
const uint16_t PIN_IR_SEND = 4;

// end pin layout for esp32

// As this program is a special purpose capture/resender, let's use a larger
// than expected buffer so we can handle very large IR messages.
// i.e. Up to 512 bits.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
const uint8_t kTimeout = 50;  // Milli-Seconds

// kFrequency is the modulation frequency all messages will be replayed at.
const uint16_t kFrequency = 38000;  // in Hz. e.g. 38kHz.

// The IR transmitter.
IRsend irsend(PIN_IR_SEND);
// The IR receiver.
IRrecv irrecv(PIN_IR_RECV, kCaptureBufferSize, kTimeout, false);

enum device_states {
  BOOT,
  CONNECTING_WIFI,
  CONNECTING_HOMEKIT,
  PAIRING_ON,
  PAIRING_OFF,
  HARD_RESET,
  SUCCESS,  // after pairing action
  GOOD,     // booted, connected to WiFi, connected to HomeKit
};

enum device_states device_state;

// void set_device_state(device_states x) {
//   LOG_D("Device state change: %s", x);
//   device_state = x;
//   handleLEDs();
// }

void setup() {
  device_state = BOOT;

  irsend.begin();
  irrecv.enableIRIn();  // Start up the IR receiver.

  Serial.begin(115200);  // Start a serial connection so you can receive HomeSpan diagnostics and control the device using HomeSpan's Command-Line Interface (CLI)

  EEPROM.begin(3000);

  delay(2000);
  while (!Serial)
    ;  //delay for Leonardo

  Serial.println();
  Serial.println("INIT");
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  handleLEDs();

  // TODO: uncomment wifi connection stuff later

  // Serial.println("CONNECTING_WIFI");

  // device_state = CONNECTING_WIFI;
  // handleLEDs();

  // // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // WiFiManager manager;
  // manager.setConnectTimeout(180);
  // manager.setConnectRetries(3);
  // bool success = manager.autoConnect("ESP32_AP", "password");

  // if (!success) {
  //   Serial.println("Failed to connect");
  // } else {
  //   Serial.println("Connected");
  // }

  //end todo move to function

  // TODO confirm wifi is connected before changing state led

  // Serial.println("CONNECTING_HOMEKIT");
  // device_state = CONNECTING_HOMEKIT;
  // handleLEDs();
  // todo test uncomment homekit due to panic
  //  my_homekit_setup();

  // TODO: uncomment in prod
  //  while (!homekit_is_paired()) {
  //    delay(10);
  //  }


  Serial.println("GOOD");
  device_state = GOOD;
  handleLEDs();
}

//==============================
// Start led handling
//==============================

// variables to hold the LED color
int Rvalue = 254, Gvalue = 1, Bvalue = 127;
;
int Rdirection = -1, Gdirection = 1, Bdirection = -1;

void rainbowLED() {
  //send PWM signal on LEDs
  analogWrite(PIN_RED, Rvalue);  // write analog signal
  analogWrite(PIN_GREEN, Gvalue);
  analogWrite(PIN_BLUE, Bvalue);

  Rvalue = Rvalue + Rdirection;  //changing values of LEDs
  Gvalue = Gvalue + Gdirection;
  Bvalue = Bvalue + Bdirection;

  //now change direction for each color if it reaches 255
  if (Rvalue >= 255 || Rvalue <= 0) {
    Rdirection = Rdirection * -1;
  }
  if (Gvalue >= 255 || Gvalue <= 0) {
    Gdirection = Rdirection * -1;
  }
  if (Bvalue >= 255 || Bvalue <= 0) {
    Bdirection = Bdirection * -1;
  }
  delay(10);  //give some delay so you can see the change
}

void testLEDs() {
  Serial.println("testLEDs");

  //  analogWrite(PIN_RED, 255);  //Glow Red
  //  analogWrite(PIN_GREEN, 0);
  //  analogWrite(PIN_BLUE, 0);
  //  delay(1000);
  //  analogWrite(PIN_RED, 0);
  //  analogWrite(PIN_GREEN, 255); //Glow Green
  //  analogWrite(PIN_BLUE, 0);
  //  delay(1000);
  //  analogWrite(PIN_RED, 0);
  //  analogWrite(PIN_GREEN, 0);
  //  analogWrite(PIN_BLUE, 255); //Glow Blue
  //  delay(1000);
  rainbowLED();
}

void handleLEDs() {
  switch (device_state) {
    // case BOOT:
    // case INIT:
    //   analogWrite(PIN_RED, 255);  //Glow Red
    //   analogWrite(PIN_GREEN, 0);
    //   analogWrite(PIN_BLUE, 0);
    //   break;
    case CONNECTING_WIFI:
      //Glow Blue
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 255);
      break;
    case CONNECTING_HOMEKIT:
      //Glow yellow
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 255);
      analogWrite(PIN_BLUE, 255);
      break;
    case PAIRING_ON:
      // flash purple
      analogWrite(PIN_RED, 162);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 255);
      delay(500);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    case PAIRING_OFF:
      // flash yellow
      analogWrite(PIN_RED, 255);
      analogWrite(PIN_GREEN, 247);
      analogWrite(PIN_BLUE, 0);
      delay(500);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    case HARD_RESET:
      // flash RED
      analogWrite(PIN_RED, 255);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      delay(250);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    case SUCCESS:
      // flash green very fast
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 255);
      analogWrite(PIN_BLUE, 0);
      delay(250);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    case GOOD:
      rainbowLED();
      break;
  }
}

//==============================
// End led handling
//==============================

//==============================
// Start button handling
//==============================

const int LONG_PRESS_TIME = 3000;   // ms
const int RESET_PRESS_TIME = 5000;  // ms

// Variables will change:
int lastButtonState;     // the previous state from the input pin
int currentButtonState;  // the current reading from the input pin
unsigned long pressedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

void handleButtonPress() {
  // read the state of the switch/button:
  currentButtonState = digitalRead(PIN_BUTTON);

  if (lastButtonState == LOW && currentButtonState == HIGH) {  // button is pressed
    Serial.println("press is detected");
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if (lastButtonState == HIGH && currentButtonState == LOW) {  // button is released
    isPressing = false;
    long pressDuration = millis() - pressedTime;
    Serial.println("press duration: ");
    Serial.print(pressDuration);

    if (pressDuration > LONG_PRESS_TIME && pressDuration <= RESET_PRESS_TIME) {
      Serial.println("A long press is detected, should go to control pairing mode");
      startIRPairing();
      isLongDetected = true;
    } else if (pressDuration > RESET_PRESS_TIME) {
      Serial.println("press > reset press time, doing action");
      // TODO: for testing, change this to dispatch the IR instead of through HA
      send_ac_on_command();

      // Serial.println("Hard reset is detected");
      // startHardReset();
    }
  }

  // save the the last state
  lastButtonState = currentButtonState;
}


void clearEEPROM() {
  // this assumes we store from 0 to 2000 position
  for (int i = 0; i < 2000; i++) {
    EEPROM.write(i, 0);  // set all to 0
  }
}

void startHardReset() {
  // clear WiFI
  // clear Homekit
  device_state = HARD_RESET;

  // TODO: migrate to homespan
  //homekit_server_reset();

  // TODO: clear esp wifi settings
  // TODO: reset eeprom
  //clearEEPROM();
  // todo restart esp
  // ESP.restart();
}


//==============================
// End button handling
//==============================

//==============================
// Start IR Send handling
//==============================


void send_ac_on_command() {
  uint16_t* irCode = readOnIRCodeFromEEPROM();
  Serial.println(sizeof(irCode));
  printUint16Array(irCode, sizeof(irCode));
  Serial.println(ESP.getFreeHeap());
  irsend.sendRaw(irCode, sizeof(irCode), kFrequency);  // Send a raw data capture at 38kHz.
  Serial.println(F("AC Switched On"));
}

void send_ac_off_command() {
  uint16_t* irCode = readOffIRCodeFromEEPROM();
  Serial.println(sizeof(irCode));
  printUint16Array(irCode, sizeof(irCode));
  Serial.println(ESP.getFreeHeap());
  irsend.sendRaw(irCode, sizeof(irCode), kFrequency);  // Send a raw data capture at 38kHz.
  Serial.println(F("AC Switched Off"));
}

//==============================
// End IR Send handling
//==============================

//==============================
// Start IR Recieve handling
//==============================
// Somewhere to store the captured message.
decode_results results;
long irLastResultTime = 0;

// todo only set on button press
bool getIR = false;

void handleIRInput() {
  bool hadResult = false;
  while (!hadResult) {
    //Continue looping until you get a complete signal received
    if (irrecv.decode(&results)) {
      // Convert the results into an array suitable for sendRaw().
      // resultToRawArray() allocates the memory we need for the array.

      // Find out how many elements are in the array.
      uint16_t* raw_array = resultToRawArray(&results);
      uint16_t rawLength = getCorrectedRawLength(&results);
      Serial.println("Captured IR");
      Serial.println("rawLength ");
      Serial.print(rawLength);

      // arbritary number, use to avoid random noise signals
      if (rawLength > 100) {
        // only mark as result recieved if the result is of a good length
        irLastResultTime = millis();
        hadResult = true;
        // The capture has stopped at this point.
        irrecv.resume();
        return;
      } else {
        Serial.println("Captured IR was not long enough, ignoring");
        irrecv.resume();
      }
    }
  }
}

void startIRPairing() {
  Serial.println("Starting IR pairing");
  // pair the on button of the remote
  device_state = PAIRING_ON;
  long startPairingOnTime = millis();
  handleIRInput();
  while (irLastResultTime < startPairingOnTime) {
    delay(10);
    // wait for input
  }
  getIR = false;
  // we have input for on button
  uint16_t* onIRCode = resultToRawArray(&results);
  uint16_t onLength = getCorrectedRawLength(&results);

  Serial.println("on ircode size");
  Serial.println(onLength);
  printUint16Array(onIRCode, sizeof(onIRCode));


  Serial.println("Captured IR for ON");
  writeIRDataToEEPROM(&onIRCode, sizeof(onIRCode), 1408, 500);

  // pair the off button of the remote
  device_state = PAIRING_OFF;
  long startPairingOffTime = millis();
  handleIRInput();
  while (irLastResultTime < startPairingOffTime) {
    delay(10);
    // wait for input
  }
  // we have input for on button
  uint16_t* offIRCode = resultToRawArray(&results);
  uint16_t offLength = getCorrectedRawLength(&results);


  Serial.println("on offIRCode size");
  Serial.println(offLength);
  printUint16Array(offIRCode, sizeof(offIRCode));

  Serial.println("Captured IR for OFF");

  writeIRDataToEEPROM(&offIRCode, sizeof(offIRCode), 2408, 500);
  device_state = SUCCESS;
  delay(1000);
  device_state = GOOD;
}

//==============================
// End IR Recieve handling
//==============================

//==============================
// Start EEPROM handling
//==============================

void writeIRDataToEEPROM(uint16_t** irCode, uint16_t irLength, int eepromStartAddress, int eepromLength) {
  if (irLength > eepromLength) {
    Serial.println("failed to write eeprom, ir length exceeded allocated eeprom storage");
    return;
  }

  eeprom_save(eepromStartAddress, *irCode);

  Serial.println("Written data to EEPROM");
}

uint16_t* readOnIRCodeFromEEPROM() {
  // TODO: set 1408 as const location
  const int addr = 1408;
  const int dataSize = 500;

  return readUint16ArrayFromEEPROM(addr, dataSize);
}

uint16_t* readOffIRCodeFromEEPROM() {
  // TODO: set 2408 as const location
  const int addr = 2408;
  const int dataSize = 500;

  return readUint16ArrayFromEEPROM(addr, dataSize);
}

uint16_t* readUint16ArrayFromEEPROM(int addr, int dataSize) {
  uint16_t* data = new uint16_t[dataSize]; // Allocate memory for the data

  for (int i = 0; i < dataSize; i++) {
    uint8_t lowByte = EEPROM.read(addr + i * 2);       // Read the low byte
    uint8_t highByte = EEPROM.read(addr + i * 2 + 1);  // Read the high byte
    data[i] = (highByte << 8) | lowByte;               // Combine the low and high bytes
  }

  return data;
}

void eeprom_save(uint addr, uint16_t* data) {
  Serial.println("saving to eeprom");
  for (uint16_t i = 0; i < sizeof(data); ++i) {
    Serial.println("step " + String(i));
    Serial.println(data[i]);

    EEPROM.write(addr + i * sizeof(uint16_t), lowByte(data[i]));       // Write the low byte
    EEPROM.write(addr + i * sizeof(uint16_t) + 1, highByte(data[i]));  // Write the high byte
  }

  EEPROM.commit();  // Commit the changes to EEPROM
}

//==============================
// End EEPROM handling
//==============================

//==============================
// Start utils
//==============================

void printUint16Array(uint16_t* array, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    Serial.print(array[i]);
    Serial.print(" ");
  }
  Serial.println();
}

//==============================
// End utils
//==============================

void loop() {

  //  testLEDs();
  handleButtonPress();
  handleLEDs();
  // todo uncomment to test
  //    homeSpan.poll();         // run HomeSpan!
  delay(10);
}
