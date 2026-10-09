// signal_generator.cpp — ESP32 LEDC square-wave excitation (stand-in
// source until ICL8038 hardware arrives). 50% duty rail-to-rail square.
// Disable parks the pin at 0V DC (safe, known state).

#include "electrical/signal_generator.h"
#include "config.h"

Esp32PwmGenerator::Esp32PwmGenerator() {}

bool Esp32PwmGenerator::begin() {
#if !SIGGEN_ENABLED
  begin_ok_ = false;
  return false;
#else
  ledcSetup(SIGGEN_LEDC_CHANNEL, (uint32_t)freq_hz_, SIGGEN_LEDC_RES_BITS);
  // 50% duty for the square wave.
  ledcWrite(SIGGEN_LEDC_CHANNEL, (1u << SIGGEN_LEDC_RES_BITS) / 2);
  ledcAttachPin(SIGGEN_PIN, SIGGEN_LEDC_CHANNEL);
  begin_ok_ = true;
  enabled_ = true;
  return true;
#endif
}

bool Esp32PwmGenerator::setFrequency(float hz) {
  if (!begin_ok_) return false;
  if (hz < SIGGEN_MIN_FREQ_HZ || hz > SIGGEN_MAX_FREQ_HZ) return false;
  freq_hz_ = hz;
  if (enabled_) {
    ledcWriteTone(SIGGEN_LEDC_CHANNEL, (uint32_t)hz);
  } else {
    ledcSetup(SIGGEN_LEDC_CHANNEL, (uint32_t)hz, SIGGEN_LEDC_RES_BITS);
  }
  return true;
}

void Esp32PwmGenerator::enable() {
  if (!begin_ok_) return;
  ledcAttachPin(SIGGEN_PIN, SIGGEN_LEDC_CHANNEL);
  ledcWriteTone(SIGGEN_LEDC_CHANNEL, (uint32_t)freq_hz_);
  enabled_ = true;
}

void Esp32PwmGenerator::disable() {
  if (!begin_ok_) return;
  ledcDetachPin(SIGGEN_PIN);
  pinMode(SIGGEN_PIN, OUTPUT);
  digitalWrite(SIGGEN_PIN, LOW);  // park at 0V DC
  enabled_ = false;
}
