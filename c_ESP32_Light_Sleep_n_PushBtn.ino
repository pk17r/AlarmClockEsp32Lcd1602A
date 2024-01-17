/*
  ESP32 Light Sleep and Push Button Functions
*/

#include <PushButtonTaps.h>

// Alarm constants
const int ALARM_END_BUTTON_PRESS_AND_HOLD_SECONDS = 25;
const unsigned long ALARM_MAX_ON_TIME_MS = 180*1000;
unsigned long secondsToAlarm = 0;

// Push Button Pin
const int BUTTON_PIN = 4;

// Push Button
PushButtonTaps pushBtn;

/*
  Esp32 light sleep function
  https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/
*/
void putEsp32ToLightSleep() {
  /*
  First we configure the wake up source
  We set our ESP32 to wake up for an external trigger.
  There are two types for ESP32, ext0 and ext1 .
  ext0 uses RTC_IO to wakeup thus requires RTC peripherals
  to be on while ext1 uses RTC Controller so doesnt need
  peripherals to be powered on.
  Note that using internal pullups/pulldowns also requires
  RTC peripherals to be turned on.
  */
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN,0); //1 = High, 0 = Low

  // add a timer to wake up ESP32 as well
  esp_sleep_enable_timer_wakeup(15000000); //15 seconds

  //Go to sleep now
  Serial.print(millis());
  Serial.println(" : Go To Light Sleep");
  //esp_deep_sleep_start();
  Serial.flush();

  // go to light sleep
  esp_light_sleep_start();

  Serial.print(millis());
  Serial.println(" : Woken Up");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if(wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
    turnBacklightOn();

  //Print the wakeup reason for ESP32
  print_wakeup_reason(wakeup_reason);

  // update time
  timeNeedsToBeUpdated = true;
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(esp_sleep_wakeup_cause_t &wakeup_reason){
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
