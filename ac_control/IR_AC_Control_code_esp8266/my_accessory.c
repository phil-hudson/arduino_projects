/*
   my_accessory.c
   Define the accessory in C language using the Macro in characteristics.h

    Created on: 2020-05-15
        Author: Mixiaoxiao (Wang Bin)
*/

#include <homekit/homekit.h>
#include <homekit/characteristics.h>

// Called to identify this accessory. See HAP section 6.7.6 Identify Routine
// Generally this is called when paired successfully or click the "Identify Accessory" button in Home APP.
void my_accessory_identify(homekit_value_t _value) {
  printf("accessory identify\n");
}

//https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266/blob/master/src/homekit/characteristics.h
/**
  Defines that the accessory contains a heater and/or cooler.

  Accessory Category:
  - 20: Heaters
  - 21: Air Conditioners

  Required Characteristics:
  - ACTIVE
  - CURRENT_TEMPERATURE
  - CURRENT_HEATER_COOLER_STATE
  - TARGET_HEATER_COOLER_STATE

  Optional Characteristics:
  - NAME
  - ROTATION_SPEED
  - TEMPERATURE_DISPLAY_UNITS
  - SWING_MODE
  - COOLING_THRESHOLD_TEMPERATURE
  - HEATING_THRESHOLD_TEMPERATURE
  - LOCK_PHYSICAL_CONTROLS
*/

// 0:Inactive, 1:Active
homekit_characteristic_t cooler_active = HOMEKIT_CHARACTERISTIC_(ON, false);

// float; min 0, max 100, step 0.1, unit celsius
homekit_characteristic_t current_temp = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 37.8);

// float; min 0, max 100, step 0.1, unit celsius
homekit_characteristic_t current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATER_COOLER_STATE, 0);

// 0: auto, 1: heat, 2:cool
homekit_characteristic_t target_state = HOMEKIT_CHARACTERISTIC_(TARGET_HEATER_COOLER_STATE, 0);

// format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "Toggle AC Power");

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_switch, .services = (homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Air Conditioner"),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "hudson"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "0000001"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
    HOMEKIT_SERVICE(SWITCH, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
      &cooler_active,
      &cha_name,
      NULL
    }),
    NULL
  }),
  NULL
};

homekit_server_config_t config = {
  .accessories = accessories,
  .password = "420-69-420"
};
