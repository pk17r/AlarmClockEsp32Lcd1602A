/*
  Preferences is Arduino EEPROM replacement library using ESP32's On-Board Non-Volatile Memory
*/

#include <Preferences.h> //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

// ESP32 EEPROM Data Access
Preferences preferences;
#define EEPROM_DATA_KEY "myfile"
const char* word_alarmHour = "alarmHour";
const char* word_alarmMin = "alarmMin";
const char* word_alarmActive = "alarmActive";
const char* word_Read = "Read ";

void esp32_preferences_eeprom_init() {
  // init saved data in EEPROM
 	getOrSetDefaultEEPROMparams(false); //readOnly = false

  Serial.print(F("ESP32 NVS Memory setup successful!"));
}

void getOrSetDefaultEEPROMparams(bool readOnly) {
  //init preference
 	preferences.begin(EEPROM_DATA_KEY, readOnly);

 	// set default alarm time if it doesn't exist in memory
  alarmHour = preferences.getUInt(word_alarmHour, ALARM_HOUR_DEFAULT); // get alarmHour or if key doesn't exist set variable to ALARM_HOUR_DEFAULT
  Serial.print(word_Read); Serial.print(char_space);
  Serial.print(word_alarmHour); Serial.print(char_space);
 	Serial.println(alarmHour);

 	alarmMin = preferences.getUInt(word_alarmMin, ALARM_MIN_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print(word_Read); Serial.print(char_space);
  Serial.print(word_alarmMin); Serial.print(char_space);
 	Serial.println(alarmMin);

 	alarmActive = preferences.getBool(word_alarmActive, ALARM_ACTIVE_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print(word_Read); Serial.print(char_space);
  Serial.print(word_alarmActive); Serial.print(char_space);
 	Serial.println(alarmActive);

  preferences.end();

}

void getEEPROMparams() {
  getOrSetDefaultEEPROMparams(true);
}

void setEEPROMparams() {
 	//init preference
 	preferences.begin(EEPROM_DATA_KEY, false);

 	preferences.putUInt(word_alarmHour, alarmHour);
 	preferences.putUInt(word_alarmMin, alarmMin);
  preferences.putBool(word_alarmActive, alarmActive);

 	Serial.print(word_Set); Serial.print(char_space);
  Serial.print(word_alarmHour); Serial.print(char_space);
 	Serial.println(alarmHour);
 	Serial.print(word_Set); Serial.print(char_space);
  Serial.print(word_alarmMin); Serial.print(char_space);
 	Serial.println(alarmMin);
 	Serial.print(word_Set); Serial.print(char_space);
  Serial.print(word_alarmActive); Serial.print(char_space);
 	Serial.println(alarmActive);

 	preferences.end();
}