/*
   wifi_info.h

    Created on: 2020-05-15
        Author: Mixiaoxiao (Wang Bin)
*/
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#ifndef WIFI_INFO_H_
#define WIFI_INFO_H_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

WiFiManager wifiManager;

const char *ssid = "Con Meo WiFi";
const char *password = "motconmeo";

void wifi_connect() {
  //WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // TODO swap in prod
    WiFi.begin(ssid, password);
  // wifiManager.autoConnect(); // create AP to configure WiFi, or autoconnect to last used
    // TODO END swap in prod


  Serial.println("WiFi connecting...");
  while (!WiFi.isConnected()) {
    delay(100);
    Serial.print(".");
  }
  Serial.print("\n");
  Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

#endif /* WIFI_INFO_H_ */
