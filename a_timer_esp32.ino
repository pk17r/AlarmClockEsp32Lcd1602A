/*
  ESP32 Hardware Timers
*/

// Hardware Timer
hw_timer_t *timeUpdateTimerPtr = NULL;
hw_timer_t *passiveBuzzerTimerPtr = NULL;

// timer alarm frequency in Hz
const int SECONDS_FREQUENCY = 1;
volatile bool timeNeedsToBeUpdated = false;

const int BUZZER_PIN = 17;
const int BUZZER_FREQUENCY = 2048;
const unsigned long BEEP_LENGTH_MS = 800;

bool buzzerOn = false;
bool _buzzerSquareWaveToggle = false;
bool _beepToggle = false;
unsigned long _beepStartTimeMs = 0;

// Timer Interrupt Service Routine
void IRAM_ATTR timeUpdateISR() {
  timeNeedsToBeUpdated = true;
}
void IRAM_ATTR passiveBuzzerTimerISR() {
  if(millis() - _beepStartTimeMs > BEEP_LENGTH_MS) {
    _beepToggle = !_beepToggle;
    _beepStartTimeMs = millis();
  }
  _buzzerSquareWaveToggle = !_buzzerSquareWaveToggle;
  digitalWrite(BUZZER_PIN, _buzzerSquareWaveToggle && _beepToggle);
}

// set timer to trigger ISR at _timer_frequency, auto reload of timer after every trigger set to true
void set_timer_frequency(hw_timer_t *timerPtr, int timerFrequency) {
  timerAlarmWrite(timerPtr, 1000000 / timerFrequency, true);
}

void timer_init() {
  // initialize timer
  timeUpdateTimerPtr = timerBegin(0, 80, true);  // using timer 0, prescaler 80 (1MHz as ESP32 is 80MHz), counting up (true)
  timerAttachInterrupt(timeUpdateTimerPtr, &timeUpdateISR, true);    //attach ISR to timer
  set_timer_frequency(timeUpdateTimerPtr, SECONDS_FREQUENCY);

  passiveBuzzerTimerPtr = timerBegin(1, 80, true);  // using timer 0, prescaler 80 (1MHz as ESP32 is 80MHz), counting up (true)
  timerAttachInterrupt(passiveBuzzerTimerPtr, &passiveBuzzerTimerISR, true);    //attach ISR to timer
  set_timer_frequency(passiveBuzzerTimerPtr, BUZZER_FREQUENCY * 2);

  Serial.println("Timer setup successful!");
}

void timer_enable(hw_timer_t *timerPtr) {
  // Timer Enable
  timerAlarmEnable(timerPtr);
}

void timer_disable(hw_timer_t *timerPtr) {
  // Timer Disable
  timerAlarmDisable(timerPtr);
}

void buzzer_enable() {
  buzzerOn = true;
  // Timer Enable
  timerAlarmEnable(passiveBuzzerTimerPtr);
}

void buzzer_disable() {
  buzzerOn = false;
  // Timer Disable
  timerAlarmDisable(passiveBuzzerTimerPtr);
  digitalWrite(BUZZER_PIN, LOW);
  _buzzerSquareWaveToggle = false;
  _beepToggle = false;
}