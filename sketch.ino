const int ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS = 25;
const unsigned long BUZZER_INTERVALS_MS = 800;

const int BUZZER_PIN = 4;
const int BUZZER_FREQUENCY = 2048;
const unsigned long BUZZER_TIMEPERIOD_US = 1000000 / BUZZER_FREQUENCY;

void setup(){
  serial_init();
  
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  // lcd 1602A init
  lcd_init();

  // init saved data in EEPROM
  esp32_preferences_eeprom_init();
  
  // get current local time from Internet
  while(!connectWiFiAndUpdateCurrentTimeFromInternet()) {
    Serial.println(F("Disconnecting and Reconnecting WiFi and attempting current time update from internet again."));
    failedToObtainTimeScreen();
    delay(3000);
  }

  //initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  button_init();

  timer_init();
  timer_enable();
}

void loop(){
  // Activate Buzzer at Alarm Time
  if(alarmActive) {
    // check during first 5 seconds of alarm time
    if(currentTimeInfo.tm_hour == alarmHour && currentTimeInfo.tm_min == alarmMin && currentTimeInfo.tm_sec <= 5) {
      buzzAlarmFn();
    }
  }

  // update local time every second (controlled by timer ISR)
  if(timeNeedsToBeUpdated) {
    updateCurrentTime();
    timeNeedsToBeUpdated = false;
    // print local time if not in Set Alarm Page
    if(!setAlarmPageActive)
      displayLocalTimeAndDate();
  }
  
  // try update time 1 hr after alarm time every day to keep current time accurate
  if(currentTimeInfo.tm_hour == alarmHour + 1 && currentTimeInfo.tm_min == alarmMin && currentTimeInfo.tm_sec < 1) {
    connectWiFiAndUpdateCurrentTimeFromInternet();
    delay(1000);  // delay 1 second so as to try updating time only once
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

void buzzAlarmFn() {
  // end Set Alarm Page flag if at all On
  setAlarmPageActive = false;
  //start buzzer!
  bool alarmEndByUser = false;
  int buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
  alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
  while(!alarmEndByUser) {
    // activate buzzer if button is not pressed by user
    if(!buttonActive()) {
      // if user lifts button press before alarm end then reset counter and re-display alarm-On screen
      if(buttonPressSecondsCounter != ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS) {
        // display Alarm On screen with seconds user needs to press and hold button to end alarm
        buttonPressSecondsCounter = ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS;
        alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
      }
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
}
