/*
  LCD 1602A with PCF8574T I/O Expander - I2C Communication
*/

/*
  Display current local time and date on display
*/
void displayLocalTimeAndDate(){
  // if today's date has changed then update both display lines
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
      lcd.print(char_zero);
    lcd.print(alarmHour);
    lcd.print(char_colon);
    if(alarmMin < 10)
      lcd.print(char_zero);
    lcd.print(alarmMin);
  }
  else
    lcd.print(word_A_OFF);

  // lcd second line -> process only if today's date is not displayed on display
  if(!currentDateOnDisplaySet) {
    lcd.setCursor(0, 1);
    lcd.print(&currentTimeInfo, "%a, %b %d %Y");
    currentDateOnDisplay_yday = currentTimeInfo.tm_yday;
    currentDateOnDisplaySet = true;
  }
}

/*
  Alarm On Screen - displayed when alarm goes off
*/
void alarmOnScreen(int countDown) {
  turnBacklightOn();
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printByte(bellId);
  lcd.print(word_Wake_Up);
  if(alarmHour < 10)
    lcd.print(char_zero);
  lcd.print(alarmHour);
  lcd.print(char_colon);
  if(alarmMin < 10)
    lcd.print(char_zero);
  lcd.print(alarmMin);
  lcd.setCursor(0, 1);
  lcd.print(word_Press_Button);
  lcd.print(countDown);
  lcd.print(char_s);
}

/*
  Good Morning Screen - displayed when alarm is ended
*/
void goodMorningScreen() {
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printByte(heartId);lcd.print(word_Good_Morning);lcd.print(&currentTimeInfo, " %a %d %b");
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

/*
  Set Alarm Page
    - three pages - set hour, set min, set alarm on/off
    - button presses to increment/fast increment/goto next page
*/
void setAlarmPage() {
  setAlarmPageActive = true;
  currentDateOnDisplaySet = false;  // to print date on display again, once time is again printed
  turnBacklightOn();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(word_ALARM);
  lcd.print(char_space);
  if(alarmHour < 10)
    lcd.print(char_zero);
  lcd.print(alarmHour);
  lcd.print(char_colon);
  if(alarmMin < 10)
    lcd.print(char_zero);
  lcd.print(alarmMin);
  lcd.print(char_space);
  if(alarmActive)
    lcd.print(word_ON);
  else
    lcd.print(word_OFF);
  lcd.setCursor(0, 1);
  lcd.print(word_Set);
  lcd.print(char_space);
  switch(setAlarmPageNumber) {
  case 0:
    lcd.print(word_Hour);
    lcd.print(char_space);
    if(setValue < 10)
      lcd.print(char_zero);
    lcd.print(setValue);
    break;
  case 1:
    lcd.print(word_Min);
    lcd.print(char_space);
    if(setValue < 10)
      lcd.print(char_zero);
    lcd.print(setValue);
    break;
  case 2:
    lcd.print(word_Alarm);
    lcd.print(char_space);
    if(setValue)
      lcd.print(word_ON);
    else
      lcd.print(word_ON);
    break;
  }
  lcd.setCursor(LCD_COLUMNS - 3, LCD_ROWS - 1);
  lcd.printByte(rightArrowId);
  lcd.setCursor(LCD_COLUMNS - 2, LCD_ROWS - 1);
  lcd.printByte(rightBigArrowId);
  lcd.setCursor(LCD_COLUMNS - 1, LCD_ROWS - 1);
  lcd.printByte(returnArrowId);
}

/*
  Process User Inputs
    - user input can be single tap, double tap, long press
    - single tap increments
    - double tap fast increments
    - long press takes user into or out of Set Alarm page
    - long press in Set Alarm Page sets value and turns page
*/
void processSetAlarmPageUserInput(byte buttonUserInput) {
  Serial.print(F("buttonUserInput ")); Serial.println(buttonUserInput);
  turnBacklightOn();
  switch(buttonUserInput) {
  case 1:  // single tap
  case 2:  // double tap
    if(setAlarmPageActive) {
      // single tap is increment
      // double tap is fast increments
      lcd.setCursor(LCD_COLUMNS - 3 + (buttonUserInput - 1), LCD_ROWS - 1);
      lcd.printByte(allWhiteId);
      switch(setAlarmPageNumber) {
      case 0: // set hour
        setValue += 1 + (buttonUserInput - 1) * 3;
        if(setValue > 23) setValue -= 24;
        break;
      case 1: // set minutes
        setValue += 5 + (buttonUserInput - 1) * 10;
        if(setValue > 59) setValue -= 60;
        break;
      case 2: // set alarm active or inactive
        setValue = !setValue;
        break;
      }
      delay(300);
      setAlarmPage();
    }
    break;
  case 3:   // long press
    if(setAlarmPageActive) {
      // return arrow inside Set Alarm Screen
      // Long Press in Set Alarm Page -> Go to next Set Alarm Page -> Save Settings -> Turn Set Alarm Page Off
      lcd.setCursor(LCD_COLUMNS - 1, LCD_ROWS - 1);
      lcd.printByte(allWhiteId);
      delay(500);
      switch(setAlarmPageNumber) {
      case 0: // set hour
        alarmHour = setValue;
        setAlarmPageNumber = 1;
        setValue = alarmMin;
        setAlarmPage();
        break;
      case 1: // set min
        alarmMin = setValue;
        setAlarmPageNumber = 2;
        setValue = alarmActive;
        setAlarmPage();
        break;
      case 2: // set alarm active or inactive
        if(setValue) alarmActive = true;
        else alarmActive = false;
        setEEPROMparams();
        setAlarmPageActive = false;
        break;
      }
    }
    else {
      // Set Alarm Welcome Screen
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print(word_SET);
      lcd.setCursor(4, 1);
      lcd.print(word_ALARM);
      lcd.print(char_space);
      lcd.printByte(bellId);
      delay(2000);
      Serial.println(F("Long Press -> Set Alarm Page On"));
      setAlarmPageNumber = 0;
      setValue = alarmHour;
      // go to Set Alarm Page
      setAlarmPage();
    }
    break;
  }
}
