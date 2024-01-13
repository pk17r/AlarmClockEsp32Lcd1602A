#include <PushButtonTapsAndPress.h>

const int ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS = 25;
const unsigned long ALARM_MAX_ON_TIME_MS = 180*1000;

const int BUTTON_PIN = 17;
//PushButtonTapsAndPress(int buttonPin, bool activeLow, bool serialPrintTapPressTimes)
PushButtonTapsAndPress pushBtn;

void buzzAlarmFn() {
  // end Set Alarm Page flag if at all On
  setAlarmPageActive = false;
  //start buzzer!
  buzzer_enable();
  bool alarmStopped = false, buzzerPausedByUser = false;
  unsigned long alarmStartTimeMs = millis();
  int buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
  alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
  while(!alarmStopped) {
    // if user presses button then pauze buzzer and start alarm end countdown!
    if(pushBtn.buttonActive()) {
      if(!buzzerPausedByUser) {
        buzzer_disable();
        buzzerPausedByUser = true;
      }
      unsigned long buttonPressStartTimeMs = millis(); //note time of button press
      // while button is pressed, display seconds countdown
      while(pushBtn.buttonActive() && !alarmStopped) {
        // display countdown to alarm off
        if(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS - (millis() - buttonPressStartTimeMs) / 1000 < buttonPressSecondsCounter) {
          buttonPressSecondsCounter--;
          alarmOnScreen(buttonPressSecondsCounter);
        }
        // end alarm after holding button for ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS
        if(millis() - buttonPressStartTimeMs > ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS * 1000) {
          alarmStopped = true;
          // good morning screen! :)
          goodMorningScreen();
        }
      }
    }
    // activate buzzer if button is not pressed by user
    if(!pushBtn.buttonActive() && !alarmStopped) {
      if(buzzerPausedByUser) {
        buzzer_enable();
        buzzerPausedByUser = false;
      }
      // if user lifts button press before alarm end then reset counter and re-display alarm-On screen
      if(buttonPressSecondsCounter != ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS) {
        // display Alarm On screen with seconds user needs to press and hold button to end alarm
        buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
        alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
      }
    }
    // if user did not stop alarm within ALARM_MAX_ON_TIME_MS, make sure to stop buzzer
    if(millis() - alarmStartTimeMs > ALARM_MAX_ON_TIME_MS) {
      buzzer_disable();
      alarmStopped = true;
    }
  }
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
}

/*
  Set Alarm Page
    - three pages - set hour, set min, set alarm on/off
    - functions to increment/decrement/goto next page
*/

void processPushButtonUserInput(ButtonTapType buttonUserInput) {
  Serial.print("buttonUserInput "); Serial.println(static_cast<int>(buttonUserInput));
  turnBacklightOn();
  switch(buttonUserInput) {
  case ButtonTapType::singleTap:  //1
  case ButtonTapType::doubleTap:  //2
    if(setAlarmPageActive) {
      // single tap is increment
      lcd.setCursor(LCD_COLUMNS - 3 + static_cast<int>(buttonUserInput) - 1, LCD_ROWS - 1);
      lcd.printByte(allWhiteId);
      if(buttonUserInput == ButtonTapType::singleTap)
        Serial.println("Single Tap in Set Alarm Page -> Increment");
      else
        Serial.println("Double Tap in Set Alarm Page -> Fast Increment");
      switch(setAlarmPageCounter) {
      case 0: 
        setValue += 1 + (static_cast<int>(buttonUserInput) - 1) * 3;
        if(setValue > 23) setValue -= 24;
        break;
      case 1:
        setValue += 5 + (static_cast<int>(buttonUserInput) - 1) * 10;
        if(setValue > 59) setValue -= 60;
        break;
      case 2:
        setValue = !setValue;
        break;
      }
      delay(300);
      setAlarmPage();
    }
    break;
  case ButtonTapType::longPress:
    if(setAlarmPageActive) {
      // return arrow inside Set Alarm Screen
      lcd.setCursor(LCD_COLUMNS - 1, LCD_ROWS - 1);
      lcd.printByte(allWhiteId);
      Serial.println("Long Press in Set Alarm Page -> Go to next Set Alarm Page -> Save Settings -> Turn Set Alarm Page Off");
      delay(500);
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
      // goto Set Alarm Page
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("SET");
      lcd.setCursor(4, 1);
      lcd.print("ALARM");
      lcd.print(" ");
      lcd.printByte(bellId);
      delay(2000);
      Serial.println("Long Press -> Set Alarm Page On");
      setAlarmPageCounter = 0;
      setValue = alarmHour;
      setAlarmPage();
    }
    break;
  }
}
