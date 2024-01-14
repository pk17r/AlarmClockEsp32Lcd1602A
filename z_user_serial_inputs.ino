/*
  Serial inputs from user
*/
void serial_init() {
  Serial.begin(115200);
}

void processSerialInput() {
  turnBacklightOn();
  char input = Serial.read();
  while(Serial.available())
    Serial.read();
  switch(input) {
    case 'b':
      if(!buzzerOn)
        buzzer_enable();
      else
        buzzer_disable();
      break;
    case 'w':
      welcomeScreen();
      delay(10000);
      break;
    case 'a':
      alarmOnScreen(ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS);
      delay(10000);
      break;
    case 'g':
      goodMorningScreen();
      break;
    case 'r':
      getEEPROMparams();
      break;
    case 's':
      Serial.println(F("Enter new alarm Hr "));
      while(Serial.available() == 0) {
        delay(1);
      };
      if(Serial.available() > 0) {
        alarmHour = Serial.parseInt();
        while(Serial.available())
          Serial.read();
      }
      Serial.println(alarmHour);
      Serial.println(F("Enter new alarm Min "));
      while(Serial.available() == 0) {
        delay(1);
      };
      if(Serial.available() > 0) {
        alarmMin = Serial.parseInt();
        while(Serial.available())
          Serial.read();
      }
      Serial.println(alarmMin);
      Serial.println(F("Enter >0 to activate alarm "));
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
