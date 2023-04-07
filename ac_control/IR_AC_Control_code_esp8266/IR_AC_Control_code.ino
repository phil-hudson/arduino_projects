#include <Arduino.h>
//#include <IRremoteESP8266.h>
//#include <IRsend.h>
#include "wifi_info.h"
//#include <IRLibRecvPCI.h>
#include <arduino_homekit_server.h>
//#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <EEPROM.h>

// CONFIGURATION PART *************************************************************************

// Set DEBUGGING for addiditonal debugging output over serial
#define DEBUGGING

// set Hostname
#define HOSTNAME "HUDSON-AC-CONTROLLER"

#define ESP8266

// END CONFIG *********************************************************************************

#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

const uint16_t BUTTON_PIN = 13;  // ESP8266 GPIO pin to use. 13 (D7).

const uint16_t redLed = 12;   // ESP8266 GPIO pin to use. 12 (D6).
const uint16_t greenLed = 2;  // ESP8266 GPIO pin to use. 14 (D5).
const uint16_t blueLed = 16;  // ESP8266 GPIO pin to use. 16 (D0).

const uint16_t kIrLedPin = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

//const uint16_t IR_reciever_pin = 2; // ESP8266 GPIO pin to use. Recommended: 2 (D4).

// The GPIO an IR detector/demodulator is connected to. Recommended: 14 (D5)
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = 14;

// TODO finish setup https://github.com/Arduino-IRremote/Arduino-IRremote/tree/master/examples/SendAndReceive
//IRrecv irrecv(IR_reciever_pin);
//IRsend irsend(kIrLed);

//IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
//IRrecvPCI myReceiver(IR_reciever_pin); //pin number for the receiver


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
IRsend irsend(kIrLedPin);
// The IR receiver.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, false);

//==============================
// HomeKit setup and loop
//==============================

//The pairing data is stored in the EEPROM address in ESP8266 Arduino core.
//The EEPROM is 4096B in ESP8266, this project uses max [0, 1408B).
//EEPROM of [1408, 4096) is safe for you to use.

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cooler_active;
extern "C" homekit_characteristic_t current_temp;
extern "C" homekit_characteristic_t current_state;
extern "C" homekit_characteristic_t target_state;

static uint32_t next_heap_millis = 0;

//Called when the switch value is changed by iOS Home APP
void cooler_active_on_setter(const homekit_value_t value) {
  bool on = value.bool_value;
  cooler_active.value = value;  //sync the value

  // TODO
  if (on == true) {
    //      int len = readOnIRCodeLenthFromEEPROM();
    // readOnIRCodeFromEEPROM();

     
    uint16_t* irCode = readOnIRCodeFromEEPROM();
        // Serial.println(printf("%u\n", (unsigned int)irCode));
//        for (int i = 0; i < sizeof(irCode); i++) {
//          Serial.println(*irCode[i]);
//        }
Serial.println(sizeof(*irCode));
Serial.println(ESP.getFreeHeap());
    irsend.sendRaw(irCode, sizeof(*irCode), kFrequency);  // Send a raw data capture at 38kHz.

    //    irsend.sendRaw(rawDataOn, RAW_DATA_LEN, kFrequency);  // Send a raw data capture at 38kHz.
    Serial.println(F("AC Switched On"));
  } else {
    //      int len = readOffIRCodeLenthFromEEPROM();
    // readOffIRCodeFromEEPROM();
    uint16_t* irCode = readOffIRCodeFromEEPROM();
//    for (int i = 0; i < sizeof(*irCode); i++) {
//          Serial.println(*irCode[i]);
//        }
    // Serial.println(printf("%u\n", (unsigned int)irCode));
    Serial.println(sizeof(*irCode));

    irsend.sendRaw(irCode, sizeof(*irCode), kFrequency);  // Send a raw data capture at 38kHz.

    //    irsend.sendRaw(rawDataOff, RAW_DATA_LEN, kFrequency);  // Send a raw data capture at 38kHz.
    Serial.println(F("AC Switched Off"));
  }
}


void current_temp_setter(const homekit_value_t value) {
  LOG_D("NO_OP: current_temp_setter Got value %d", value.float_value);
}

void current_state_setter(const homekit_value_t value) {
  LOG_D("NO_OP: current_state_setter. Got value %d", value.int_value);
}

void target_state_setter(const homekit_value_t value) {
  LOG_D("NO_OP: target_state_setter. Got value %d", value.int_value);
}

void my_homekit_setup() {
  //Add the .setter function to get the switch-event sent from iOS Home APP.
  //The .setter should be added before arduino_homekit_setup.
  //HomeKit sever uses the .setter_ex internally, see homekit_accessories_init function.
  //Maybe this is a legacy design issue in the original esp-homekit library,
  //and I have no reason to modify this "feature".

  //report the switch value to HomeKit if it is changed (e.g. by a physical button)
  //bool switch_is_on = true/false;
  //cha_switch_on.value.bool_value = switch_is_on;
  //homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);

  cooler_active.setter = cooler_active_on_setter;
  current_temp.setter = current_temp_setter;
  current_state.setter = current_state_setter;
  target_state.setter = target_state_setter;
  arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}

//==============================
// End HomeKit setup and loop
//==============================

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

#ifdef ESP8266
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else   // ESP8266
  Serial.begin(115200, SERIAL_8N1);
#endif  // ESP8266

  delay(2000);
  while (!Serial)
    ;  //delay for Leonardo


  // DEBUGGING

    
//    uint16_t* irCode = readOnIRCodeFromEEPROM();
        // Serial.println(printf("%u\n", (unsigned int)irCode));
//        for (int i = 0; i < sizeof(irCode); i++) {
//          Serial.println(irCode[i]);
//        }
//Serial.println(sizeof(irCode));

  // DEBUGGING

  Serial.println();
  Serial.println("INIT");
  pinMode(BUTTON_PIN, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  handleLEDs();

  //  homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  Serial.println("CONNECTING_WIFI");

  device_state = CONNECTING_WIFI;
  handleLEDs();
  wifi_connect();  // in wifi_info.h

  // TODO confirm wifi is connected before changing state led

  Serial.println("CONNECTING_HOMEKIT");
  device_state = CONNECTING_HOMEKIT;
  handleLEDs();
  my_homekit_setup();
  // TODO: uncomment in prod
  //  while (!homekit_is_paired()) {
  //    delay(10);
  //  }


  Serial.println("GOOD");
  device_state = GOOD;
  handleLEDs();
}

//void setup() {
//  device_state = BOOT;
//  irsend.begin();
//  irrecv.enableIRIn();  // Start up the IR receiver.
//
//
//#ifdef ESP8266
//  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
//#else   // ESP8266
//  Serial.begin(115200, SERIAL_8N1);
//#endif  // ESP8266
//
//  delay(2000);
//  while (!Serial);  //delay for Leonardo
//
//  pinMode(BUTTON_PIN, INPUT);
//  pinMode(redLed, OUTPUT);
//  pinMode(greenLed, OUTPUT);
//  pinMode(blueLed, OUTPUT);
//  handleLEDs();
//
//  //  homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
//  // set_device_state(CONNECTING_WIFI);
//  LOG_D("Device state change: %s", CONNECTING_WIFI);
//  device_state = CONNECTING_WIFI;
//  handleLEDs();
//  wifi_connect();  // in wifi_info.h
//
//  // TODO confirm wifi is connected before changing state led
//
//  // set_device_state(CONNECTING_HOMEKIT);
//    LOG_D("Device state change: %s", CONNECTING_HOMEKIT);
//  device_state = CONNECTING_HOMEKIT;
//  handleLEDs();
//
//  my_homekit_setup();
//  // TODO: uncomment in prod
//  //  while (!homekit_is_paired()) {
//  //    delay(10);
//  //  }
//
//  // set_device_state(GOOD);
//    LOG_D("Device state change: %s", GOOD);
//  device_state = GOOD;
//  handleLEDs();
//}

//==============================
// Start led handling
//==============================

// variables to hold the LED color
int Rvalue = 254, Gvalue = 1, Bvalue = 127;
;
int Rdirection = -1, Gdirection = 1, Bdirection = -1;

void rainbowLED() {
  //send PWM signal on LEDs
  analogWrite(redLed, Rvalue);  // write analog signal
  analogWrite(greenLed, Gvalue);
  analogWrite(blueLed, Bvalue);

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

  //  analogWrite(redLed, 255);  //Glow Red
  //  analogWrite(greenLed, 0);
  //  analogWrite(blueLed, 0);
  //  delay(1000);
  //  analogWrite(redLed, 0);
  //  analogWrite(greenLed, 255); //Glow Green
  //  analogWrite(blueLed, 0);
  //  delay(1000);
  //  analogWrite(redLed, 0);
  //  analogWrite(greenLed, 0);
  //  analogWrite(blueLed, 255); //Glow Blue
  //  delay(1000);
  rainbowLED();
}

void handleLEDs() {
  switch (device_state) {
    // case BOOT:
    // case INIT:
    //   analogWrite(redLed, 255);  //Glow Red
    //   analogWrite(greenLed, 0);
    //   analogWrite(blueLed, 0);
    //   break;
    case CONNECTING_WIFI:
      //Glow Blue
      analogWrite(redLed, 0);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 255);
      break;
    case CONNECTING_HOMEKIT:
      //Glow yellow
      analogWrite(redLed, 0);
      analogWrite(greenLed, 255);
      analogWrite(blueLed, 255);
      break;
    case PAIRING_ON:
      // flash purple
      analogWrite(redLed, 162);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 255);
      delay(500);
      analogWrite(redLed, 0);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 0);
      break;
    case PAIRING_OFF:
      // flash yellow
      analogWrite(redLed, 255);
      analogWrite(greenLed, 247);
      analogWrite(blueLed, 0);
      delay(500);
      analogWrite(redLed, 0);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 0);
      break;
    case HARD_RESET:
      // flash RED
      analogWrite(redLed, 255);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 0);
      delay(250);
      analogWrite(redLed, 0);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 0);
      break;
    case SUCCESS:
      // flash green very fast
      analogWrite(redLed, 0);
      analogWrite(greenLed, 255);
      analogWrite(blueLed, 0);
      delay(250);
      analogWrite(redLed, 0);
      analogWrite(greenLed, 0);
      analogWrite(blueLed, 0);
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

const int LONG_PRESS_TIME = 5000;    // ms
const int RESET_PRESS_TIME = 10000;  // ms

// Variables will change:
int lastButtonState;     // the previous state from the input pin
int currentButtonState;  // the current reading from the input pin
unsigned long pressedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

void handleButtonPress() {
  // read the state of the switch/button:
  currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == LOW && currentButtonState == HIGH) {  // button is pressed
    Serial.println("press is detected");
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if (lastButtonState == HIGH && currentButtonState == LOW) {  // button is released
    isPressing = false;
    long pressDuration = millis() - pressedTime;
    LOG_D("press duration: %d",
          pressDuration);

    if (pressDuration > LONG_PRESS_TIME && pressDuration <= RESET_PRESS_TIME) {
      Serial.println("A long press is detected, should go to control pairing mode");
      startIRPairing();
      isLongDetected = true;
    } else if (pressDuration > RESET_PRESS_TIME) {
      Serial.println("Hard reset is detected");
      startHardReset();
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

  homekit_server_reset();
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
      uint16_t* raw_array = resultToRawArray(&results);
      // Find out how many elements are in the array.
      uint16_t length = getCorrectedRawLength(&results);

      Serial.println("Captured IR");
      Serial.println(resultToHumanReadableBasic(&results));
      Serial.println(length);

      // arbritary number, use to avoid random noise signals
      if (length > 100) {
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
    delay(50);
  }
}

int irOnLengthEEPROMLocation = 4000;
int irOffLengthEEPROMLocation = 4002;

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
    Serial.println("on ircode size");

  Serial.println(sizeof(onIRCode));
  uint16_t onLength = getCorrectedRawLength(&results);
  Serial.println("Captured IR for ON");
  Serial.println(onLength);
  EEPROM.write(irOnLengthEEPROMLocation, onLength);
  writeIRDataToEEPROM(&onIRCode, onLength, 1408, 1000);

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
      Serial.println("on offIRCode size");

  Serial.println(sizeof(offIRCode));
  uint16_t offLength = getCorrectedRawLength(&results);
  Serial.println("Captured IR for OFF");
  Serial.println(offLength);

  EEPROM.write(irOffLengthEEPROMLocation, offLength);
  writeIRDataToEEPROM(&offIRCode, offLength, 2408, 1000);
  device_state = SUCCESS;
  delay(1000);
  device_state = GOOD;
}


// TODO - TEST
void writeIRDataToEEPROM(uint16_t** irCode, uint16_t irLength, int eepromStartAddress, int eepromLength) {
  if (irLength > eepromLength) {
    Serial.println("failed to write eeprom, ir length exceeded allocated eeprom storage");
    return;
  }

  eeprom_save(0, *irCode, irLength);

  Serial.println("Written data to EEPROM");
}

uint16_t* readOnIRCodeFromEEPROM() {
      Serial.println("start");

// TODO errors within this bit
//Exception 9: LoadStoreAlignmentCause: Load or store to an unaligned address
  uint16_t* data;
//  int len = EEPROM.read(irOnLengthEEPROMLocation);
//   EEPROM.begin(len);
   EEPROM.begin(1000);

    Serial.println("allocate");

  EEPROM.get(1408, data);
//  eeprom_load(1408, data, 1000);

  return data;
}

uint16_t* readOffIRCodeFromEEPROM() {
  uint16_t* data;
  int len = EEPROM.read(irOffLengthEEPROMLocation);
   EEPROM.begin(len);
  
  eeprom_load(2408, data, len);

  return data;
}

//
// https://stackoverflow.com/questions/66077317/how-to-encapsulate-eeprom-put-and-eeprom-get-in-own-functions-arduino-c-esp
//

void eeprom_save(uint addr, uint16_t *data, uint len) {
  EEPROM.begin(len);
  for (uint i = 0; i < len; i++) {
    EEPROM.put(addr + i, data[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void eeprom_load(uint addr, uint16_t *data, uint len) {
  EEPROM.begin(len);
  for (uint i = 0; i < len; i++) {
    EEPROM.get(addr + i, data[i]);
  }
  EEPROM.end();
}

//
// https://stackoverflow.com/questions/66077317/how-to-encapsulate-eeprom-put-and-eeprom-get-in-own-functions-arduino-c-esp
//


int readOffIRCodeLenthFromEEPROM() {
  int len;
  EEPROM.get(irOffLengthEEPROMLocation, len);
  return len;
}

int readOnIRCodeLenthFromEEPROM() {
  int len;
  EEPROM.get(irOnLengthEEPROMLocation, len);
  return len;
}

//==============================
// End IR Recieve handling
//==============================



void loop() {

  //  testLEDs();
  handleButtonPress();
  handleLEDs();
  my_homekit_loop();
  delay(10);
}
