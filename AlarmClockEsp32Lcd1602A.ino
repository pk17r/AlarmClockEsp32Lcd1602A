/*
  Powerful Alarm Clock with Long Press Alarm End Button to actually make you Wake Up! :)
  
  Prashant Kumar
  
  Components Required:
  Microcontroller - ESP32
  Buzzer - Passive Buzzer - KSSG1203-42 - Rated Frequency 2048Hz, 3-5V, 35mA
  Display - LCD 1602A with PCF8574T I/O Expander - I2C Communication
  Level Shifter - TXS0108E - to connect 5V LCD I2C to 3.3V ESP32 I2C as ESP32 pins are not 5V tolerant
  Button - 1x Tactile Button Pulled Up with a 10K resistor pull-up to 3.3V and a 100nF Decoupling Capacitor to ground
  MOSFET - 1x N7000 NPN MOSFET - Powers Buzzer with 5V and driven by ESP32 Buzzer Drive Pin to Gate
  LED - 1x 5mm LED that flashes along with Buzzer, driven by ESP32 Buzzer Drive Pin
  Resistor - 1x 10Ohm - In series with Buzzer to contain current to 35mA - 5V to Buzzer+
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  Features:
  - remembers set alarm time on reboot
  - drives a 90dB passive buzzer at rated frequency using timer of ESP32 and a mosfet at 5V
  - updates displayed time every second
  - uses only 1 button to set/enable/disable alarm and end active alarm
  - when alarm goes off, user needs to press and hold button for 25 seconds to end alarm, otherwise it will start buzzing again
  - updates time using WiFi from NTC server at start of microcontroller and everyday 1 hr after the set alarm time
  
*/

