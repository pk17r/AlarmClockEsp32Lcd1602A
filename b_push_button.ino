/*
  Push Button
    - setup single push button
    - process taps as 
      - no tap
      - single tap
      - double tap
      - long press
*/

const int BUTTON_PIN = 17;

// types of button presses
enum class ButtonTapType {  noTap, 
                            singleTap, 
                            doubleTap, 
                            longPress
};

// last button status
volatile ButtonTapType buttonTap = ButtonTapType::noTap;

void button_init() {
    //initialize button
  pinMode(BUTTON_PIN, INPUT);
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
      buttonTap = ButtonTapType::longPress;
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