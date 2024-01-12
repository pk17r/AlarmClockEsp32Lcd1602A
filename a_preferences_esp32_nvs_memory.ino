// Preferences is Arduino EEPROM replacement library using ESP32's On-Board Non-Volatile Memory

#include <Preferences.h> //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

// ESP32 EEPROM Data Access
Preferences preferences;
#define EEPROM_DATA_KEY "myfile"

void esp32_preferences_eeprom_init() {
  // init saved data in EEPROM
 	getOrSetDefaultEEPROMparams(false); //readOnly = false

  Serial.print("ESP32 NVS Memory setup successful!");
}

void getOrSetDefaultEEPROMparams(bool readOnly) {
  //init preference
 	preferences.begin(EEPROM_DATA_KEY, readOnly);

 	// set default alarm time if it doesn't exist in memory
  alarmHour = preferences.getUInt("alarmHour", ALARM_HOUR_DEFAULT); // get alarmHour or if key doesn't exist set variable to ALARM_HOUR_DEFAULT
 	Serial.print("Read alarmHour = ");
 	Serial.println(alarmHour);

 	alarmMin = preferences.getUInt("alarmMin", ALARM_MIN_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print("Read alarmMin = ");
 	Serial.println(alarmMin);

 	alarmActive = preferences.getBool("alarmActive", ALARM_ACTIVE_DEFAULT); // get alarmMin or if key doesn't exist set variable to ALARM_MIN_DEFAULT
 	Serial.print("Read alarmActive = ");
 	Serial.println(alarmActive);

  preferences.end();

}

void getEEPROMparams() {
  getOrSetDefaultEEPROMparams(true);
}

void setEEPROMparams() {
 	//init preference
 	preferences.begin(EEPROM_DATA_KEY, false);

 	preferences.putUInt("alarmHour", alarmHour);
 	preferences.putUInt("alarmMin", alarmMin);
  preferences.putBool("alarmActive", alarmActive);

 	Serial.print("Set alarmHour = ");
 	Serial.println(alarmHour);
 	Serial.print("Set alarmMin = ");
 	Serial.println(alarmMin);
 	Serial.print("Set alarmActive = ");
 	Serial.println(alarmActive);
 	
 	preferences.end();
}