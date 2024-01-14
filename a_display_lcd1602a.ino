/*
  LCD 1602A with PCF8574T I/O Expander - I2C Communication
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// LCD Display constants
const int LCD_ADDRESS = 0x27;
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;
const unsigned long BACKLIGHT_TURNOFF_AFTER_MS = 60*1000;

// special lcd characters -> we only have 8 locations 0-7
uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t bellId = 0x00;
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t heartId = 0x01;
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t duckId = 0x02;
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t checkId = 0x03;
// uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
// uint8_t crossId = 0x04;
uint8_t rightBigArrow[8] = {  0x08,  0x04,  0x02,  0x1F,  0x02,  0x04,  0x08,  0x00};
uint8_t rightBigArrowId = 0x04;
uint8_t returnArrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};
uint8_t returnArrowId = 0x05;
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

// display characters words
const char* char_colon = ":";
const char* char_zero = "0";
const char* char_space = " ";
const char* char_s = "s";
const char* word_ALARM = "ALARM";
const char* word_Alarm = "Alarm";
const char* word_A_OFF = "A-OFF";
const char* word_Hour = "Hour";
const char* word_Min = "Min";
const char* word_ON = "ON";
const char* word_OFF = "OFF";
const char* word_Set = "Set";
const char* word_SET = "SET";
const char* word_Good_Morning = " Good Morning!";
const char* word_Wake_Up = " WAKE-UP! ";
const char* word_Press_Button = "Press Button ";
const char* word_Long_Press = " Long Press ";

// LCD VARIABLES
bool backlightOn = false;
unsigned long backlightTurnedOnAtMs = 0;

// keeping track of today's date to update display's second line only on date change
int currentDateOnDisplay_yday = 0;  // tm_yday Number of days elapsed since last January 1, between 0 and 365 inclusive.  https://sourceware.org/newlib/libc.html#Timefns
bool currentDateOnDisplaySet = false;

// Alarm variables
unsigned int alarmHour = 0;
unsigned int alarmMin = 0;
bool alarmActive = false;
const unsigned int ALARM_HOUR_DEFAULT = 6;
const unsigned int ALARM_MIN_DEFAULT = 30;
const bool ALARM_ACTIVE_DEFAULT = false;

// Set Alarm Page variables
bool setAlarmPageActive = false; // to set alarm
int setAlarmPageNumber = 0; // 0 - set hr, 1 - set min, 2 - set alarm active/inactive
int setValue = 0;

void lcd_init() {
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
  lcd.createChar(rightBigArrowId, rightBigArrow);
  lcd.createChar(returnArrowId, returnArrow);
  lcd.createChar(smileyId, smiley);
  lcd.createChar(allWhiteId, allWhite);
  
  //display welcome screen
  welcomeScreen();

  Serial.print(F("LCD 1602A setup successful!"));
}

void welcomeScreen() {
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(char_space); lcd.printByte(heartId); lcd.print(word_Long_Press); lcd.printByte(smileyId); lcd.print(char_space);
  lcd.setCursor(0, 1);
  lcd.printByte(bellId); lcd.print(char_space); lcd.printByte(bellId); lcd.print(char_space); lcd.printByte(bellId); lcd.print(char_space); lcd.print(word_Alarm); lcd.print(char_space); lcd.print(char_space); lcd.printByte(bellId); lcd.print(char_space); lcd.printByte(bellId);
}

void failedToObtainTimeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Failed to obtain"));
  lcd.setCursor(0, 1);
  lcd.print(F("time!"));
}

void turnBacklightOn() {
  // turn on LCD backlight
  lcd.backlight();
  backlightOn = true;
  backlightTurnedOnAtMs = millis();
}
