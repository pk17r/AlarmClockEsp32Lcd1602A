/*
  Powerful Alarm Clock with Long Press Alarm End Button to actually make you Wake Up! :)
  
  Prashant Kumar
  
  Components Required:
  Microcontroller - ESP32
  Buzzer - Passive Buzzer - KSSG1203-42 - Rated Frequency 2048Hz, 3-5V, 35mA
  Display - LCD 1602A with PCF8574T I/O Expander - I2C Communication
  Button - 1x Tactile Button Pulled Up with a 10K resistor pull-up to 3.3V and a 100nF Decoupling Capacitor to ground
  MOSFET - 1x N7000 NPN MOSFET - Powers Buzzer with 5V and driven by ESP32 Buzzer Drive Pin to Gate
  LED - 1x 5mm LED that flashes along with Buzzer, driven by ESP32 Buzzer Drive Pin
  Resistor - 1x 10Ohm - In series with Buzzer to contain current to 35mA - 5V to Buzzer+
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
*/

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

// print local time function decleration
int printLocalTime();

// tm time stuct to keep current time
struct tm currentTimeInfo;


#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// LCD Display constants and variables
const int LCD_ADDRESS = 0x27;
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;
const unsigned long BACKLIGHT_TURNOFF_AFTER_MS = 60*1000;
bool backlightOn = false;
unsigned long backlightTurnedOnAtMs = 0;

uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t bellId = 0x00;
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t heartId = 0x01;
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t duckId = 0x02;
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t checkId = 0x03;
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t crossId = 0x04;
uint8_t retArrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};
uint8_t retArrowId = 0x05;
// characters created using https://maxpromer.github.io/LCD-Character-Creator/
uint8_t smiley[8]  = {0x02,  0x01,  0x09,  0x01,  0x01,  0x09,  0x01,  0x02};
uint8_t smileyId = 0x06;
uint8_t allWhite[8] = {  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F,  0x1F};
uint8_t allWhiteId = 0x07;
uint8_t rightArrowId = 0x7E;
uint8_t leftArrowId = 0x7F;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);


#include <Preferences.h> //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

// ESP32 EEPROM Data Access
Preferences preferences;
#define EEPROM_DATA_KEY "myfile"

unsigned int alarmHour = 0;
unsigned int alarmMin = 0;
bool alarmActive = false;
const unsigned int ALARM_HOUR_DEFAULT = 6;
const unsigned int ALARM_MIN_DEFAULT = 30;
const bool ALARM_ACTIVE_DEFAULT = false;

const int BUTTON_PIN = 27;
const int ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS = 25;
const unsigned long BUZZER_INTERVALS_MS = 800;

// types of button presses
enum class ButtonTapType { noTap, singleTap, doubleTap, longTap };
volatile ButtonTapType buttonTap = ButtonTapType::noTap;

const int BUZZER_PIN = 4;
const int BUZZER_FREQUENCY = 2048;
const unsigned long BUZZER_TIMEPERIOD_US = 1000000 / BUZZER_FREQUENCY;

bool setAlarmPageActive = false; // to set alarm
int setAlarmPageCounter = 0; // 0 - hr, 1 - min, 2 - alarm active
int setValue = 0;

// keeping track of today's date to update display's second line only on date change
int currentDateOnDisplay_yday = 0;  // tm_yday Number of days elapsed since last January 1, between 0 and 365 inclusive.  https://sourceware.org/newlib/libc.html#Timefns
bool currentDateOnDisplaySet = false;

// Hardware Timer to tick every 1 second
hw_timer_t *printLocalTimeTimer = NULL;

volatile bool runPrintLocalTimeFn = false;

// Timer Interrupt Service Routine
void IRAM_ATTR printLocalTimeISR() {
  runPrintLocalTimeFn = true;
}

void setup(){
  Serial.begin(115200);

  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  turnBacklightOn();
  // create lcd characters
  lcd.createChar(bellId, bell);
  lcd.createChar(heartId, heart);
  //lcd.createChar(duckId, duck);
  //lcd.createChar(checkId, check);
  //lcd.createChar(crossId, cross);
  lcd.createChar(retArrowId, retArrow);
  lcd.createChar(smileyId, smiley);
  lcd.createChar(allWhiteId, allWhite);
  //display welcome screen
  welcomeScreen();

  // init saved data in EEPROM
 	getOrSetDefaultEEPROMparams(false); //readOnly = false

  // get current local time from Internet
  int attemptsWiFi = 0;
  while(connectWiFiAndUpdateCurrentTimeFromInternet() || attemptsWiFi > 3) {
    attemptsWiFi++;
    Serial.println("Disconnecting and Reconnecting WiFi and attempting time update again.");
    delay(3000);
  }

  //initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  //initialize button
  pinMode(BUTTON_PIN, INPUT);

  // initialize timer
  printLocalTimeTimer = timerBegin(0, 80, true);  // using timer 0, prescaler 80 (1MHz as ESP32 is 80MHz), counting up (true)
  timerAttachInterrupt(printLocalTimeTimer, &printLocalTimeISR, true);    //attach ISR to timer
  timerAlarmWrite(printLocalTimeTimer, 1000000, true);      // set timer to trigger ISR every second, auto reload of timer after every trigger set to true

  // Timer Enable
  timerAlarmEnable(printLocalTimeTimer);
}

void loop(){
  // Activate Buzzer at Alarm Time
  if(alarmActive) {
    // check during first 5 seconds of alarm time
    if(currentTimeInfo.tm_hour == alarmHour && currentTimeInfo.tm_min == alarmMin && currentTimeInfo.tm_sec <= 5) {
      buzzAlarmFn();
    }
  }

  // print local time every second if not in Set Alarm Page
  if(!setAlarmPageActive && runPrintLocalTimeFn) {
    runPrintLocalTimeFn = false;
    printLocalTime();
  }
  
  // try update time 1 hr after alarm time every day to keep time accurate
  if(currentTimeInfo.tm_hour == alarmHour + 1 && currentTimeInfo.tm_min == alarmMin && currentTimeInfo.tm_sec <= 5) {
    connectWiFiAndUpdateCurrentTimeFromInternet();
    delay(5000);
  }

  // turn off display backlight after BACKLIGHT_TURNOFF_AFTER_MS of being On
  if(backlightOn && millis() - backlightTurnedOnAtMs > BACKLIGHT_TURNOFF_AFTER_MS) {
    lcd.noBacklight();
    backlightOn = false;
  }

  // check for button press user input
  checkBtnPress();
  if(buttonTap != ButtonTapType::noTap) {
    processButtonTap();
  }

  // serial inputs and processing for debugging and development
  if(Serial.available()) {
    processSerialInput();
  }
}

int connectWiFiAndUpdateCurrentTimeFromInternet() {
  int returnVal = 1;
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
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

void welcomeScreen() {
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" "); lcd.printByte(heartId); lcd.print(" Long Press "); lcd.printByte(smileyId); lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.printByte(bellId); lcd.print(" "); lcd.printByte(bellId); lcd.print(" "); lcd.printByte(bellId); lcd.print(" Alarm  "); lcd.printByte(bellId); lcd.print(" "); lcd.printByte(bellId);
}

int printLocalTime(){
  if(!getLocalTime(&currentTimeInfo)){
    Serial.println("Failed to obtain time");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to obtain");
    lcd.setCursor(0, 1);
    lcd.print("time!");
    return 1;
  }

  // clear LCD if today's date has changed
  if(currentTimeInfo.tm_yday != currentDateOnDisplay_yday)
    currentDateOnDisplaySet = false;

  // clear LCD if today's date is not displayed on LCD
  if(!currentDateOnDisplaySet)
    lcd.clear();

  // lcd first line
  lcd.setCursor(0, 0);
  lcd.print(&currentTimeInfo, "%I:%M:%S%P ");
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

  // lcd second line -> process only if today's date is not displayed on display
  if(!currentDateOnDisplaySet) {
    lcd.setCursor(0, 1);
    lcd.print(&currentTimeInfo, "%a, %b %d %Y");
    currentDateOnDisplay_yday = currentTimeInfo.tm_yday;
    currentDateOnDisplaySet = true;
  }
  return 0;
}

void setAlarmPage() {
  setAlarmPageActive = true;
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
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
  switch(setAlarmPageCounter) {
  case 0:
    lcd.print("Set Hour ");
    if(setValue < 10)
      lcd.print("0");
    lcd.print(setValue);
    break;
  case 1:
    lcd.print("Set Min ");
    if(setValue < 10)
      lcd.print("0");
    lcd.print(setValue);
    break;
  case 2:
    lcd.print("Set Alarm ");
    if(setValue)
      lcd.print("ON");
    else
      lcd.print("OFF");
    break;
  }
  lcd.setCursor(LCD_COLUMNS - 3, LCD_ROWS - 1);
  lcd.printByte(rightArrowId);
  lcd.setCursor(LCD_COLUMNS - 2, LCD_ROWS - 1);
  lcd.printByte(leftArrowId);
  lcd.setCursor(LCD_COLUMNS - 1, LCD_ROWS - 1);
  lcd.printByte(retArrowId);
}

void buzzAlarmFn() {
  // Timer Disable
  timerAlarmDisable(printLocalTimeTimer);
  // end Set Alarm Page flag if at all On
  setAlarmPageActive = false;
  //start buzzer!
  bool alarmEndByUser = false;
  while(!alarmEndByUser) {
    // display Alarm On screen with seconds user needs to press and hold button to end alarm
    alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
    // activate buzzer if button is not pressed by user
    if(!buttonActive()) {
      unsigned long timeStartMs = millis();
      // buzz for one interval or until button is pressed
      while((millis() - timeStartMs < BUZZER_INTERVALS_MS) && !buttonActive()) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(BUZZER_TIMEPERIOD_US / 2);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(BUZZER_TIMEPERIOD_US / 2);
      }
      timeStartMs = millis();
      // stay silent for one interval or until button is pressed
      while((millis() - timeStartMs < BUZZER_INTERVALS_MS) && !buttonActive()) {
        delay(1);
      }
    }
    // if user presses button then start alarm end countdown!
    if(buttonActive()) {
      unsigned long buttonPressStartTimeMs = millis(); //note time of button press
      int buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
      // while button is pressed, display seconds countdown
      while(buttonActive() && !alarmEndByUser) {
        // display countdown to alarm off
        if(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS - (millis() - buttonPressStartTimeMs) / 1000 < buttonPressSecondsCounter) {
          buttonPressSecondsCounter--;
          alarmOnScreen(buttonPressSecondsCounter);
        }
        // end alarm after holding button for ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS
        if(millis() - buttonPressStartTimeMs > ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS * 1000) {
          // good morning screen! :)
          goodMorningScreen();
          alarmEndByUser = true;
        }
      }
    }
  }
  // Alarm ended by user!
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  // Timer re-enable
  timerAlarmEnable(printLocalTimeTimer);
}

void alarmOnScreen(int countDown) {
  turnBacklightOn();
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printByte(bellId);
  lcd.print(" WAKE-UP! ");
  if(alarmHour < 10)
    lcd.print("0");
  lcd.print(alarmHour);
  lcd.print(":");
  if(alarmMin < 10)
    lcd.print("0");
  lcd.print(alarmMin);
  lcd.setCursor(0, 1);
  lcd.print("Press Button ");
  lcd.print(countDown);
  lcd.print("s");
}

void goodMorningScreen() {
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printByte(heartId);lcd.print(" Good Morning!");lcd.print(&currentTimeInfo, " %a %d %b");
  lcd.setCursor(0, 1);
  for(int i = 0; i < 7; i++) {
    lcd.print(" ");
    lcd.printByte(smileyId);
    lcd.print(" ");
    lcd.printByte(heartId);
  }
  delay(1000);
  for(int i = 0; i < 10; i++) {
    lcd.scrollDisplayLeft();
    delay(1000);
  }
}

bool buttonActive() {
  return !digitalRead(BUTTON_PIN);  //active low
}

void checkBtnPress() {
  // set buttonTap to noTap
  buttonTap = ButtonTapType::noTap;
  unsigned long btnPressStartTimeMs = millis();
  // check for button press and note how much time it is pressed
  while(buttonActive()) {
    // it is at least a single tap
    buttonTap = ButtonTapType::singleTap;
    delay(1);
    if(millis() - btnPressStartTimeMs > 650) {
      // it is a long tap
      buttonTap = ButtonTapType::longTap;
      // set display change inside while loop itself to indicate accepted user input
      if(!setAlarmPageActive) {
        // goto Set Alarm Page
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("SET");
        lcd.setCursor(4, 1);
        lcd.print("ALARM");
        lcd.print(" ");
        lcd.printByte(bellId);
        delay(2000);
      }
      else {
        // return arrow inside Set Alarm Screen
        lcd.setCursor(LCD_COLUMNS - 1, LCD_ROWS - 1);
        lcd.printByte(allWhiteId);
        delay(500);
      }
    }
  }

  // if single Tap then check for double Tap
  if(buttonTap == ButtonTapType::singleTap) {
    btnPressStartTimeMs = millis();
    // wait for 300 milliseconds with no tap to see if user agains taps
    while(!buttonActive() && millis() - btnPressStartTimeMs < 300) {
      delay(1);
    }
    while(buttonActive()) {
      delay(1);
      // it is a double Tap
      buttonTap = ButtonTapType::doubleTap;
      // set display change inside while loop itself to indicate accepted user input
      if(setAlarmPageActive) {
        // double tap is decrement or left arrow inside Set Alarm Screen
        lcd.setCursor(LCD_COLUMNS - 2, LCD_ROWS - 1);
        lcd.printByte(allWhiteId);
        delay(300);
      }
    }
    // set display change here itself to indicate accepted user input
    if(setAlarmPageActive && buttonTap == ButtonTapType::singleTap) {
      // single tap is increment or right arrow inside Set Alarm Screen
      lcd.setCursor(LCD_COLUMNS - 3, LCD_ROWS - 1);
      lcd.printByte(allWhiteId);
      delay(300);
    }
  }
}

void processButtonTap() {
  Serial.print("buttonTap "); Serial.println(static_cast<int>(buttonTap));
  turnBacklightOn();
  switch(buttonTap) {
  case ButtonTapType::singleTap:
    if(setAlarmPageActive) {
      Serial.println("Short Press in Set Alarm Page -> Increment setting");
      switch(setAlarmPageCounter) {
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
      setAlarmPage();
    }
    break;
  case ButtonTapType::doubleTap:
    if(setAlarmPageActive) {
      Serial.println("Double Press in Set Alarm Page -> Decrement setting");
      switch(setAlarmPageCounter) {
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
      setAlarmPage();
    }
    break;
  case ButtonTapType::longTap:
    if(setAlarmPageActive) {
      Serial.println("Long Press in Set Alarm Page -> Go to next Set Alarm Page -> Save Settings -> Turn Set Alarm Page Off");
      switch(setAlarmPageCounter) {
      case 0:
        alarmHour = setValue;
        setAlarmPageCounter = 1;
        setValue = alarmMin;
        setAlarmPage();
        break;
      case 1:
        alarmMin = setValue;
        setAlarmPageCounter = 2;
        setValue = alarmActive;
        setAlarmPage();
        break;
      case 2:
        if(setValue)
          alarmActive = true;
        else
          alarmActive = false;
        setEEPROMparams();
        setAlarmPageActive = false;
        break;
      }
    }
    else {
      Serial.println("Long Press -> Set Alarm Page On");
      setAlarmPageCounter = 0;
      setValue = alarmHour;
      setAlarmPage();
    }
    break;
  }
}

void turnBacklightOn() {
  // turn on LCD backlight
  lcd.backlight();
  backlightOn = true;
  backlightTurnedOnAtMs = millis();
}

void processSerialInput() {
  turnBacklightOn();
  char input = Serial.read();
  while(Serial.available())
    Serial.read();
  switch(input) {
    case 'w':
      welcomeScreen();
      delay(5000);
      break;
    case 'a':
      alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
      delay(5000);
      break;
    case 'g':
      goodMorningScreen();
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
void setEEPROMparams() {
 	//init preference
 	preferences.begin(EEPROM_DATA_KEY, false);

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
  getOrSetDefaultEEPROMparams(true);
}

void getOrSetDefaultEEPROMparams(bool readOnly) {
  //init preference
 	preferences.begin(EEPROM_DATA_KEY, readOnly);

 	// set default alarm time if it doesn't exist in memory
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

}