// Hardware Timer
hw_timer_t *timeUpdateTimer = NULL;
volatile bool timeNeedsToBeUpdated = false;

// timer alarm frequency in Hz
const int SECONDS_FREQUENCY = 1;

// Timer Interrupt Service Routine
void IRAM_ATTR timeUpdateISR() {
  timeNeedsToBeUpdated = true;
}

// set timer to trigger ISR at _timer_frequency, auto reload of timer after every trigger set to true
void set_timer_frequency(int _timer_frequency) {
  timerAlarmWrite(timeUpdateTimer, 1000000 / _timer_frequency, true);
}

void timer_init() {
  // initialize timer
  timeUpdateTimer = timerBegin(0, 80, true);  // using timer 0, prescaler 80 (1MHz as ESP32 is 80MHz), counting up (true)
  timerAttachInterrupt(timeUpdateTimer, &timeUpdateISR, true);    //attach ISR to timer
  set_timer_frequency(SECONDS_FREQUENCY);

  Serial.print("Timer setup successful!");
}

void timer_enable() {
  // Timer Enable
  timerAlarmEnable(timeUpdateTimer);
}

void timer_disable() {
  // Timer Disable
  timerAlarmDisable(timeUpdateTimer);
}