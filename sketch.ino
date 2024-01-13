void processPushButtonUserInput(ButtonTapType buttonUserInput);

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

  // initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // initialize push button
  pushBtn.pushButtonSetup(BUTTON_PIN, true, false);

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
  ButtonTapType buttonUserInput = pushBtn.checkButtonStatus();
  if(buttonUserInput != ButtonTapType::noTap && !backlightOn)
    turnBacklightOn();
  else if(buttonUserInput != ButtonTapType::noTap)
    processPushButtonUserInput(buttonUserInput);
  

  // serial inputs and processing for debugging and development
  if(Serial.available()) {
    processSerialInput();
  }
}
