
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
