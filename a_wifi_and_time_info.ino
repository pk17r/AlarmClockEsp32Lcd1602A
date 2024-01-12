/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-date-time-ntp-client-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include "time.h"
#include "secrets_file.h"

const char* SSID     = WIFI_SSID;
const char* PASSWORD = WIFI_PASSWORD;

const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = -8*60*60;
const int   DAYLIGHT_OFFSET_SEC = 60*60;

// tm time stuct to keep current time
struct tm currentTimeInfo;

// get and update current local time from Internet
bool connectWiFiAndUpdateCurrentTimeFromInternet() {
  bool success = false;

  // Connect to Wi-Fi
  Serial.print(F("Connecting to "));
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  int attemptsToConnectToWiFi = 1;
  while (WiFi.status() != WL_CONNECTED) {
    attemptsToConnectToWiFi++;
    if(attemptsToConnectToWiFi > 5) {
      Serial.println();
      Serial.println(F("Could not connect to WiFi!"));
      break;
    }
    delay(500);
    Serial.print(F("."));
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(F("WiFi connected."));
    delay(1000);

    // Init and get the time
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    int attemptsToUpdateTimeFromNtcServer = 1;
    while(true) {
      Serial.print(F("Update time from NTC Server attempt "));
      Serial.println(attemptsToUpdateTimeFromNtcServer);
      //returnVal = displayLocalTimeAndDate();
      if(updateCurrentTime()) {
        Serial.println(F("Update time from NTC Server successful!"));
        success = true;
        break;
      }
      if(attemptsToUpdateTimeFromNtcServer > 10) {
        Serial.println(F("Update time from NTC Server Unsuccessful!"));
        break;
      }
      attemptsToUpdateTimeFromNtcServer++;
      delay(1000);
    }

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println(F("WiFi Disconnected."));
  }
  return success;
}

bool updateCurrentTime() {
  return getLocalTime(&currentTimeInfo);
}