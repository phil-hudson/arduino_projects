#include <Arduino.h>
#include <HomeSpan.h>
#include <IRsend.h>
#include <IRrecv.h> // https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/IRrecv.h
#include <EEPROM.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
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
const uint16_t PIN_IR_SEND = 32;

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

//==============================
// HomeKit setup and loop
//==============================


// todo migrate
//
////Called when the switch value is changed by iOS Home APP
//void cooler_active_on_setter(const homekit_value_t value) {
//  bool on = value.bool_value;
//  cooler_active.value = value;  //sync the value
//
//  // TODO
//  if (on == true) {
//    //      int len = readOnIRCodeLenthFromEEPROM();
//    // readOnIRCodeFromEEPROM();
//
//
//    uint16_t* irCode = readOnIRCodeFromEEPROM();
//        // Serial.println(printf("%u\n", (unsigned int)irCode));
////        for (int i = 0; i < sizeof(irCode); i++) {
////          Serial.println(*irCode[i]);
////        }
//Serial.println(sizeof(*irCode));
//Serial.println(ESP.getFreeHeap());
//    irsend.sendRaw(irCode, sizeof(*irCode), kFrequency);  // Send a raw data capture at 38kHz.
//
//    //    irsend.sendRaw(rawDataOn, RAW_DATA_LEN, kFrequency);  // Send a raw data capture at 38kHz.
//    Serial.println(F("AC Switched On"));
//  } else {
//    //      int len = readOffIRCodeLenthFromEEPROM();
//    // readOffIRCodeFromEEPROM();
//    uint16_t* irCode = readOffIRCodeFromEEPROM();
////    for (int i = 0; i < sizeof(*irCode); i++) {
////          Serial.println(*irCode[i]);
////        }
//    // Serial.println(printf("%u\n", (unsigned int)irCode));
//    Serial.println(sizeof(*irCode));
//
//    irsend.sendRaw(irCode, sizeof(*irCode), kFrequency);  // Send a raw data capture at 38kHz.
//
//    //    irsend.sendRaw(rawDataOff, RAW_DATA_LEN, kFrequency);  // Send a raw data capture at 38kHz.
//    Serial.println(F("AC Switched Off"));
//  }
//}
//
//
//void current_temp_setter(const homekit_value_t value) {
//  LOG_D("NO_OP: current_temp_setter Got value %d", value.float_value);
//}
//
//void current_state_setter(const homekit_value_t value) {
//  LOG_D("NO_OP: current_state_setter. Got value %d", value.int_value);
//}
//
//void target_state_setter(const homekit_value_t value) {
//  LOG_D("NO_OP: target_state_setter. Got value %d", value.int_value);
//}

// TODO migrate to homespan
void my_homekit_setup() {
  // The HomeSpan library creates a global object named "homeSpan" that encapsulates all HomeSpan functionality.
  // The begin() method is used to initialize HomeSpan and start all HomeSpan processes.

  // The first two parameters are Category and Name, which are used by HomeKit to configure the icon and name
  // of the device shown in the Home App when initially pairing a HomeSpan device with your iPhone.

  // In addition, the Name you choose below will be used as the "default name" for all Accessory Tiles.  When you first
  // pair the device, the Home App will display this default name and allow you to change it (for each Accessory Tile)
  // before pairing is complete.  However, even after the device is paired you can always change the name of any
  // Accessory Tile directly from the Home App via the set-up screen for any Tile.

  // IMPORTANT: The Name you choose below MUST BE UNIQUE across all your HomeSpan devices!

  homeSpan.begin(Category::Lighting, "HomeSpan LightBulb");  // initializes a HomeSpan device named "HomeSpan Lightbulb" with Category set to Lighting

  // Next, we construct a simple HAP Accessory Database with a single Accessory containing 3 Services,
  // each with their own required Characteristics.

  new SpanAccessory();                              // Begin by creating a new Accessory using SpanAccessory(), no arguments needed

  new Service::AccessoryInformation();            // HAP requires every Accessory to implement an AccessoryInformation Service

  // The only required Characteristic for the Accessory Information Service is the special Identify Characteristic.  It takes no arguments:

  new Characteristic::Identify();               // Create the required Identify Characteristic

  // The Accessory Information Service also includes these four OPTIONAL Characteristics.  They perform no function and are for
  // informational purposes only --- their values are displayed in HomeKit's setting panel for each Accessory.  Feel free
  // to uncomment the lines and implement any combination of them, or none at all.

  //      new Characteristic::Manufacturer("HomeSpan");   // Manufacturer of the Accessory (arbitrary text string, and can be the same for every Accessory)
  //      new Characteristic::SerialNumber("123-ABC");    // Serial Number of the Accessory (arbitrary text string, and can be the same for every Accessory)
  //      new Characteristic::Model("120-Volt Lamp");     // Model of the Accessory (arbitrary text string, and can be the same for every Accessory)
  //      new Characteristic::FirmwareRevision("0.9");    // Firmware of the Accessory (arbitrary text string, and can be the same for every Accessory)

  // *NOTE* HAP requires that the Accessory Information Service always be instantiated BEFORE any other Services, which is why we created it first.

  // Now that the required "informational" Services have been defined, we can finally create our Light Bulb Service

  new Service::LightBulb();                       // Create the Light Bulb Service
  new Characteristic::On();                       // This Service requires the "On" Characterstic to turn the light on and off

  // That's all that's needed to define a database from scratch, including all required HAP elements, to control a single lightbulb.
  // Of course this sketch does not yet contain any code to implement the actual operation of the light - there is nothing to
  // turn on and off.  But you'll still see a Light Bulb tile show up in your Home App with an ability to toggle it on and off.
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

  Serial.begin(115200);       // Start a serial connection so you can receive HomeSpan diagnostics and control the device using HomeSpan's Command-Line Interface (CLI)

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

  //  homekit_storage_reset(); // to remove the previous HomeKit pairing storage when you first run this new HomeKit example
  Serial.println("CONNECTING_WIFI");

  device_state = CONNECTING_WIFI;
  handleLEDs();

  // todo move to function:


  WiFiManager manager;
  bool success = manager.autoConnect("ESP32_AP", "password");

  if (!success) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("Connected");
  }
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

// FIXME: problem looks like it is only sending the first index of the array from irCode
  void send_ac_on_command() {
    uint16_t* irCode = readOnIRCodeFromEEPROM();
    Serial.println(sizeof(*irCode));
    printUint16Array(irCode, sizeof(irCode));
    Serial.println(ESP.getFreeHeap());
    irsend.sendRaw(irCode, sizeof(*irCode), kFrequency);  // Send a raw data capture at 38kHz.
    Serial.println(F("AC Switched On"));
 }

  //==============================
  // Start button handling
  //==============================

  const int LONG_PRESS_TIME = 3000;    // ms
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

        // TODO this isnt working
        // TODO use results.value
        // Find out how many elements are in the array.
        uint16_t* raw_array = resultToRawArray(&results);
        uint16_t length = getCorrectedRawLength(&results);
        Serial.println("Captured IR");
        Serial.println("length ");
        Serial.print(length);

        // Serial.println("raw array sizeof");
        // Serial.println(sizeof(raw_array));

        // arbritary number, use to avoid random noise signals
        // NOTE: currently set to 50 for testing, used to be 100
        if (length > 50) { 
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
     uint16_t onLength = getCorrectedRawLength(&results);

//  uint64_t* onIRCode = &results.value;
    Serial.println("on ircode size");
    Serial.println(onLength);
    printUint16Array(onIRCode, sizeof(onIRCode));


//    uint16_t onLength = getCorrectedRawLength(&results);
    Serial.println("Captured IR for ON");
//    Serial.println(onLength);
  //  EEPROM.write(irOnLengthEEPROMLocation, onLength);
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
    
    //uint16_t offLength = getCorrectedRawLength(&results);
    Serial.println("Captured IR for OFF");
    //Serial.println(offLength);

    //EEPROM.write(irOffLengthEEPROMLocation, offLength);
    // todo uncomment write
    writeIRDataToEEPROM(&offIRCode, sizeof(offIRCode), 2408, 500);
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

    eeprom_save(eepromStartAddress, *irCode);

    Serial.println("Written data to EEPROM");
  }

uint16_t* readOnIRCodeFromEEPROM() {
  const int addr = 1408;

const int dataSize = 500;
  uint16_t* data = new uint16_t[dataSize]; // Allocate memory for the data
  Serial.println("read");
      // TODO: set 1408 as const location


    for (int i = 0; i < dataSize; i++) {
    uint8_t lowByte = EEPROM.read(addr + i * 2); // Read the low byte
    uint8_t highByte = EEPROM.read(addr + i * 2 + 1); // Read the high byte
    data[i] = (highByte << 8) | lowByte; // Combine the low and high bytes
  }
    Serial.println("done");

  return data;
}

  uint16_t* readOffIRCodeFromEEPROM() {
      const int addr = 2408;

const int dataSize = 500;

    uint16_t*  data = new uint16_t[dataSize];  
    // TODO: set 2408 as const location

    for (int i = 0; i < dataSize; i++) {
    uint8_t lowByte = EEPROM.read(addr + i * 2); // Read the low byte
    uint8_t highByte = EEPROM.read(addr + i * 2 + 1); // Read the high byte
    data[i] = (highByte << 8) | lowByte; // Combine the low and high bytes
  }

    return data;
  }

  void eeprom_save(uint addr, uint16_t *data) {
    Serial.println("saving to eeprom");
  for (uint16_t i = 0; i < sizeof(data); ++i) {
    Serial.println("step " + String(i));
        Serial.println(data[i]);

    EEPROM.write(addr + i * sizeof(uint16_t), lowByte(data[i])); // Write the low byte
    EEPROM.write(addr + i * sizeof(uint16_t) + 1, highByte(data[i])); // Write the high byte
  }

  EEPROM.commit(); // Commit the changes to EEPROM

    // EEPROM.put(addr, data);
    // EEPROM.commit();
  }



  //==============================
  // End IR Recieve handling
  //==============================


void printUint16Array(uint16_t* array, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    Serial.print(array[i]);
    Serial.print(" ");
  }
  Serial.println();
}


  void loop() {

    //  testLEDs();
    handleButtonPress();
    handleLEDs();
// todo uncomment to test
//    homeSpan.poll();         // run HomeSpan!
    delay(10);
  }
