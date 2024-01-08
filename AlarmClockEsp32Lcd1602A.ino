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

int printLocalTime();

struct tm timeinfo;

/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
const int LCD_ADDRESS = 0x27;
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Task Handle for core 0
TaskHandle_t TaskCore0;

const int BUZZER_PIN = 4;
const int BUZZER_FREQUENCY = 2048;
const unsigned long BUZZER_TIMEPERIOD_US = 1000000 / BUZZER_FREQUENCY;

#include <Preferences.h> //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

Preferences preferences;
unsigned int alarmHour = 0;
unsigned int alarmMin = 0;
bool alarmActive = false;
const unsigned int ALARM_HOUR_DEFAULT = 6;
const unsigned int ALARM_MIN_DEFAULT = 30;
const bool ALARM_ACTIVE_DEFAULT = false;

const int BUTTON_PIN = 27;
const int SNOOZE_HOLD_SEC = 25;
const unsigned long BUZZER_INTERVALS_MS = 800;

bool buzzerON = false;

bool settingsMode = false; // to set alarm
int settingsModeCounter = 0; // 0 - hr, 1 - min, 2 - alarm active
int setValue = 0;

int currentHr = 0, currentMin = 0, currentSec = 0; // Hr in 24 hr format
int yday = 0;
bool currentDayOnScreenSet = false;

bool backlightOn = false;
unsigned long backlightTurnedOnAtMs = 0;
const unsigned long BACKLIGHT_TURNOFF_AFTER_MS = 20000;

void setup(){
  Serial.begin(115200);

  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  turnBacklightOn();
  
  int attemptsWiFi = 0;
  while(updateTimeFromInternet() || attemptsWiFi > 2) {
    attemptsWiFi++;
    Serial.println("Disconnecting and Reconnecting WiFi and attempting time update again.");
    delay(3000);
  }

  // set default alarm time if it doesn't exist in memory
  //init preference
 	preferences.begin("myfile", false);

 	alarmHour = preferences.getUInt("alarmHour", ALARM_HOUR_DEFAULT); // get alarmHour or if key doesn't exist set variable to ALARM_HOUR_DEFAULT
 	Serial.print("Read alarmHour = ");
 	Serial.println(alarmHour);
 	
 	alarmMin = preferences.getUInt("alarmMin", ALARM_MIN_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print("Read alarmMin = ");
 	Serial.println(alarmMin);
 	
 	alarmActive = preferences.getBool("alarmActive", ALARM_ACTIVE_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print("Read alarmActive = ");
 	Serial.println(alarmActive);
 	
  preferences.end();

  //initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  //initialize alarm off pin
  pinMode(BUTTON_PIN, INPUT);

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    TimeAndLCDUpdateTask,   /* Task function. */
                    "TimeAndLCDUpdateTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskCore0,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500);
}

int updateTimeFromInternet() {
  int returnVal = 1;
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  int attempts = 1;
  while (WiFi.status() != WL_CONNECTED) {
    attempts++;
    if(attempts > 5) {
      Serial.println("Could not connect to WiFi!");
      break;
    }
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected.");
    delay(1000);

    // Init and get the time
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    attempts = 1;
    while(true) {
      Serial.print("Update time from internet attempt ");
      Serial.println(attempts);
      returnVal = printLocalTime();
      if(returnVal == 0) {
        Serial.println("Update time from internet successful!");
        break;
      }
      if(attempts > 10) {
        Serial.println("Update time from internet Unsuccessful!");
        break;
      }
      attempts++;
      delay(1000);
    }

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi Disconnected.");
  }
  return returnVal;
}

void loop(){
  // Serial.print("loop() running on core ");
  // Serial.print(xPortGetCoreID());

  // Activate Buzzer at Alarm Time
  if(alarmActive && currentDayOnScreenSet) {
    // check during first 5 seconds of alarm time
    if(currentHr == alarmHour && currentMin == alarmMin && currentSec <= 5) {
      //start buzzer!
      buzzerON = true;
    }
  }

  //update time 1 hr after alarm
  if(currentHr == alarmHour + 1 && currentMin == alarmMin && currentSec <= 5) {
    updateTimeFromInternet();
    delay(5000);
  }

  if(backlightOn && millis() - backlightTurnedOnAtMs > BACKLIGHT_TURNOFF_AFTER_MS) {
    lcd.noBacklight();
    backlightOn = false;
  }

  // single press
  if(!settingsMode && buttonActive()) {
    turnBacklightOn();
  }

  if(!buzzerON && buttonActive()) {
    // Alarm ON -> buzzerON case is handled in core0 loop
    turnBacklightOn();
    bool longPress = false, doublePress = false;
    checkBtnPress(longPress, doublePress);
    Serial.print("longPress ");
    Serial.print(longPress);
    Serial.print("   doublePress ");
    Serial.println(doublePress);

    if(!settingsMode && longPress) {
      Serial.println("Long Press -> Settings Mode On");
      settingsMode = true;
      settingsModeCounter = 0;
      setValue = alarmHour;
      settingsScreen();
    }
    else if(settingsMode && longPress) {
      Serial.println("Long Press in Settings Mode -> Go to next Settings -> Save Settings -> Turn Settings Mode Off");
      switch(settingsModeCounter) {
      case 0:
        alarmHour = setValue;
        settingsModeCounter = 1;
        setValue = alarmMin;
        settingsScreen();
        break;
      case 1:
        alarmMin = setValue;
        settingsModeCounter = 2;
        setValue = alarmActive;
        settingsScreen();
        break;
      case 2:
        if(setValue)
          alarmActive = true;
        else
          alarmActive = false;
        setEEPROMparams();
        settingsMode = false;
        break;
      }
    }
    else if(settingsMode && !longPress && !doublePress) {
      Serial.println("Short Press in Settings Mode -> Increment setting");
      switch(settingsModeCounter) {
      case 0:
        if(setValue < 23)
          setValue++;
        else
          setValue = 0;
        break;
      case 1:
        if(setValue < 55)
          setValue = min(setValue + 5, 55);
        else
          setValue = 0;
        break;
      case 2:
        if(setValue)
          setValue = 0;
        else
          setValue = 1;
        break;
      }
      settingsScreen();
    }
    else if(settingsMode && !longPress && doublePress) {
      Serial.println("Double Press in Settings Mode -> Decrement setting");
      switch(settingsModeCounter) {
      case 0:
        if(setValue > 0)
          setValue--;
        else
          setValue = 23;
        break;
      case 1:
        if(setValue > 0)
          setValue = max(setValue - 5, 0);
        else
          setValue = 55;
        break;
      case 2:
        if(setValue)
          setValue = 0;
        else
          setValue = 1;
        break;
      }
      settingsScreen();
    }

  }

  if(Serial.available()) {
    turnBacklightOn();
    char input = Serial.read();
    while(Serial.available())
      Serial.read();
    switch(input) {
      case 'b':
        Serial.print("Buzzer ");
        if(!buzzerON)
        {
          Serial.println("ON");
          buzzerON = true;
        }
        else
        {
          Serial.println("OFF");
          buzzerON = false;
          delay(50);
          digitalWrite(BUZZER_PIN, LOW);
        }
        break;
      case 'r':
        getEEPROMparams();
        break;
      case 's':
        Serial.println("Enter new alarm Hr ");
        while(Serial.available() == 0) {
          delay(1);
        };
        if(Serial.available() > 0) {
          alarmHour = Serial.parseInt();
          while(Serial.available())
            Serial.read();
        }
        Serial.println(alarmHour);
        Serial.println("Enter new alarm Min ");
        while(Serial.available() == 0) {
          delay(1);
        };
        if(Serial.available() > 0) {
          alarmMin = Serial.parseInt();
          while(Serial.available())
            Serial.read();
        }
        Serial.println(alarmMin);
        Serial.println("Enter >0 to activate alarm ");
        while(Serial.available() == 0) {
          delay(1);
        };
        if(Serial.available() > 0) {
          alarmActive = Serial.parseInt();
          while(Serial.available())
            Serial.read();
        }
        Serial.println(alarmActive);
        setEEPROMparams();
        break;
    }
  }
}

//Task1code: blinks an LED every 1000 ms
void TimeAndLCDUpdateTask( void * pvParameters ){
  Serial.print("TimeAndLCDUpdateTask running on core ");
  Serial.println(xPortGetCoreID());

  for(;;) {
    if(!settingsMode)
    {
      if(!buzzerON) {
        printLocalTime();
        delay(1000);
        if(settingsMode)
          settingsScreen();
      }
      else {
        alarmOnScreen(SNOOZE_HOLD_SEC);
        if(!buttonActive()) {
          unsigned long timeStartMs = millis();
          while((millis() - timeStartMs < BUZZER_INTERVALS_MS) && !buttonActive()) {
            digitalWrite(BUZZER_PIN, HIGH);
            delayMicroseconds(BUZZER_TIMEPERIOD_US / 2);
            digitalWrite(BUZZER_PIN, LOW);
            delayMicroseconds(BUZZER_TIMEPERIOD_US / 2);
          }
          timeStartMs = millis();
          while((millis() - timeStartMs < BUZZER_INTERVALS_MS) && !buttonActive()) {
            delay(1);
          }
        }
        if(buttonActive()) {
          unsigned long snoozeTimeStartMs = millis(); //note time of snooze press
          int snoozeHoldSecondsCounter = SNOOZE_HOLD_SEC;
          while(buttonActive()) {
            //end alarm after holding snooze for SNOOZE_HOLD_SEC
            if(millis() - snoozeTimeStartMs > SNOOZE_HOLD_SEC * 1000) {
              //good morning screen! :)
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Good Morning!");
              for(int i = 0; i < 10; i++) {
                if(i % 3 == 0) {
                  lcd.setCursor(13, 0);
                  lcd.print(":) ");
                  lcd.setCursor(0, 1);
                  lcd.print(":) :) :) :) :) :");
                }
                else if(i % 3 == 1) {
                  lcd.setCursor(13, 0);
                  lcd.print(" :)");
                  lcd.setCursor(0, 1);
                  lcd.print(" :) :) :) :) :) ");
                }
                else {
                  lcd.setCursor(13, 0);
                  lcd.print("  :");
                  lcd.setCursor(0, 1);
                  lcd.print(") :) :) :) :) :)");
                }
                delay(1000);
              }
              lcd.clear();
              buzzerON = false;
              break;
            }
            // display countdown to alarm off
            if(SNOOZE_HOLD_SEC - (millis() - snoozeTimeStartMs) / 1000 < snoozeHoldSecondsCounter) {
              snoozeHoldSecondsCounter--;
              alarmOnScreen(snoozeHoldSecondsCounter);
            }
          }
        }
      }
    }
    else {
      currentDayOnScreenSet = false;  // to do lcd clear() once date is again printed
      backlightTurnedOnAtMs = millis();   // keep backlight On
      delay(10);
    }
  } 
}

bool buttonActive() {
  return !digitalRead(BUTTON_PIN);
}

void checkBtnPress(bool &longPress, bool &doublePress) {
  unsigned long firstBtnPressStartTimeMs = millis();
  // note how much time btn is pressed
  while(buttonActive()) {
    delay(1);
    if(millis() - firstBtnPressStartTimeMs > 650) {
      longPress = true;
      lcd.setCursor(15, 1);
      lcd.print("L");
    }
  }
  unsigned long firstBtnPressTimeMs = millis() - firstBtnPressStartTimeMs;
  doublePress = false;
  if(!longPress) {
    // check for second btn press
    firstBtnPressStartTimeMs = millis();
    while(!buttonActive() && millis() - firstBtnPressStartTimeMs < 300) {
      delay(1);
    }
    unsigned long secondBtnPressStartTimeMs = millis();
    while(buttonActive()) {
      delay(1);
      doublePress = true;
      if(settingsMode) {
        lcd.setCursor(15, 1);
        lcd.print("D");
      }
    }
  }
}

void turnBacklightOn() {
  // turn on LCD backlight
  lcd.backlight();
  backlightOn = true;
  backlightTurnedOnAtMs = millis();
}

void settingsScreen() {
  turnBacklightOn();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm ");
  if(alarmHour < 10)
    lcd.print("0");
  lcd.print(alarmHour);
  lcd.print(":");
  if(alarmMin < 10)
    lcd.print("0");
  lcd.print(alarmMin);
  if(alarmActive)
    lcd.print(" ON");
  else
    lcd.print(" OFF");
  lcd.setCursor(0, 1);
  switch(settingsModeCounter) {
  case 0:
    lcd.print("Set Hr to ");
    lcd.print(setValue);
    break;
  case 1:
    lcd.print("Set Min to ");
    lcd.print(setValue);
    break;
  case 2:
    lcd.print("Set Alarm ");
    if(setValue)
      lcd.print(" ON");
    else
      lcd.print(" OFF");
    break;
  }
}

void alarmOnScreen(int countDown) {
  turnBacklightOn();
  currentDayOnScreenSet = false;  // to do lcd clear() once date is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAKE-UP! ");
  if(alarmHour < 10)
    lcd.print("0");
  lcd.print(alarmHour);
  lcd.print(":");
  if(alarmMin < 10)
    lcd.print("0");
  lcd.print(alarmMin);
  lcd.setCursor(0, 1);
  lcd.print("Press Snooze ");
  lcd.print(countDown);
  lcd.print("s");
}

int printLocalTime(){
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to obtain");
    lcd.setCursor(0, 1);
    lcd.print("time!");
    return 1;
  }

  currentHr = timeinfo.tm_hour;
  currentMin = timeinfo.tm_min;
  currentSec = timeinfo.tm_sec;
  // Serial.print("Time ");
  // Serial.print(currentHr);
  // Serial.print(":");
  // Serial.print(currentMin);
  // Serial.print(":");
  // Serial.println(currentSec);
  // Serial.print("timeinfo.tm_yday ");
  // Serial.print(timeinfo.tm_yday);
  // Serial.print("yday ");
  // Serial.print(yday);
  // Serial.print("currentDayOnScreenSet ");
  // Serial.println(currentDayOnScreenSet);

  // not clearing lcd all the time
  if(static_cast<int>(timeinfo.tm_yday) != yday)
    currentDayOnScreenSet = false;
  if(!currentDayOnScreenSet)
    lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(&timeinfo, "%I:%M:%S%P ");
  if(alarmActive) {
    if(alarmHour < 10)
      lcd.print("0");
    lcd.print(alarmHour);
    lcd.print(":");
    if(alarmMin < 10)
      lcd.print("0");
    lcd.print(alarmMin);
  }
  else
    lcd.print("A-OFF");
  if(!currentDayOnScreenSet) {
    lcd.setCursor(0, 1);
    lcd.print(&timeinfo, "%a, %b %d %Y");
    yday = timeinfo.tm_yday;
    currentDayOnScreenSet = true;
  }
  return 0;
}

void setEEPROMparams() {
 	//init preference
 	preferences.begin("myfile", false);

 	preferences.putUInt("alarmHour", alarmHour);
 	preferences.putUInt("alarmMin", alarmMin);
  preferences.putBool("alarmActive", alarmActive);

 	Serial.print("Set alarmHour = ");
 	Serial.println(alarmHour);
 	Serial.print("Set alarmMin = ");
 	Serial.println(alarmMin);
 	Serial.print("Set alarmActive = ");
 	Serial.println(alarmActive);
 	
 	preferences.end();
}

void getEEPROMparams() {
 	preferences.begin("myfile", true);

 	alarmHour = preferences.getUInt("alarmHour", 0);
 	Serial.print("Read alarmHour = ");
 	Serial.println(alarmHour);
 	
 	alarmMin = preferences.getUInt("alarmMin", 0);
 	Serial.print("Read alarmMin = ");
 	Serial.println(alarmMin);
 	
 	alarmActive = preferences.getBool("alarmActive", false);
 	Serial.print("Read alarmActive = ");
 	Serial.println(alarmActive);
 	
 	preferences.end();
}