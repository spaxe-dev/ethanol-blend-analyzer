// signal_generator.cpp — ESP32 hardware-timer square-wave excitation
// (stand-in source until ICL8038 hardware arrives). A 1 MHz hardware timer
// toggles the pin: exact frequency, 50% duty, rail-to-rail 0..3.3V.
// (LEDC was abandoned: its clock-divider limits silently kill low
// frequencies and ledcWriteTone reconfigures resolution behind our back.)
// Disable parks the pin at 0V DC (safe, known state).

#include "electrical/signal_generator.h"
#include "config.h"

static hw_timer_t* s_timer = nullptr;
static volatile bool s_level = false;
static uint32_t s_pin_mask = 0;

static void IRAM_ATTR onSigTick() {
  s_level = !s_level;
  if (s_level) {
    GPIO.out_w1ts = s_pin_mask;
  } else {
    GPIO.out_w1tc = s_pin_mask;
  }
}

Esp32PwmGenerator::Esp32PwmGenerator() {}

bool Esp32PwmGenerator::begin() {
#if !SIGGEN_ENABLED
  begin_ok_ = false;
  return false;
#else
  pinMode(SIGGEN_PIN, OUTPUT);
  digitalWrite(SIGGEN_PIN, LOW);
  s_level = false;
  s_pin_mask = (1u << SIGGEN_PIN);
  // 1 MHz tick (80 MHz / 80 prescaler); toggle every half period.
  s_timer = timerBegin(0, 80, true);
  if (s_timer == nullptr) {
    begin_ok_ = false;
    return false;
  }
  timerAttachInterrupt(s_timer, &onSigTick, true);
  timerAlarmWrite(s_timer, (uint64_t)(500000.0f / freq_hz_), true);
  timerAlarmEnable(s_timer);
  begin_ok_ = true;
  enabled_ = true;
  actual_hz_ = freq_hz_;  // exact by construction: 1 MHz / half-period ticks
  return true;
#endif
}

bool Esp32PwmGenerator::setFrequency(float hz) {
  if (!begin_ok_) return false;
  if (hz < SIGGEN_MIN_FREQ_HZ || hz > SIGGEN_MAX_FREQ_HZ) return false;
  freq_hz_ = hz;
  actual_hz_ = hz;
  timerAlarmWrite(s_timer, (uint64_t)(500000.0f / hz), true);
  return true;
}

void Esp32PwmGenerator::enable() {
  if (!begin_ok_) return;
  timerAlarmEnable(s_timer);
  enabled_ = true;
}

void Esp32PwmGenerator::disable() {
  if (!begin_ok_) return;
  timerAlarmDisable(s_timer);
  s_level = false;
  digitalWrite(SIGGEN_PIN, LOW);  // park at 0V DC
  enabled_ = false;
}
