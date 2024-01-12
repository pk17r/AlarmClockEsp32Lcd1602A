/*
  Set Alarm Page
    - three pages - set hour, set min, set alarm on/off
    - functions to increment/decrement/goto next page
*/

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
  case ButtonTapType::longPress:
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
