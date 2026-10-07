// adc.cpp — ADS1115 via Adafruit_ADS1X15 (Phase 1)
// Gain/range come from config.h. Never probe arbitrary voltages: keep inputs
// within GND..VDD (3.3V-powered => ~3.6V abs max). Single-ended CH test only.

#include "sensors/adc.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

static Adafruit_ADS1115 ads;

Ads1115Reader::Ads1115Reader() {}

bool Ads1115Reader::begin() {
#if !ADS1115_ENABLED
  healthy_ = false;
  return false;
#else
  // Wire.begin() is done in main.cpp before all drivers.
  ads.setGain(ADS1115_GAIN);
  ads.setDataRate(ADS1115_DATARATE);
  bool ok = ads.begin(ADS1115_I2C_ADDRESS);
  begin_ok_ = ok;
  healthy_ = ok;
  valid_ = false;
  last_sample_ms_ = 0;
  return ok;
#endif
}

void Ads1115Reader::update() {
#if !ADS1115_ENABLED
  return;
#else
  if (!begin_ok_) return;
  unsigned long now = millis();
  if (now - last_sample_ms_ < ADS1115_SAMPLING_INTERVAL_MS && last_sample_ms_ != 0) {
    return;
  }
  last_sample_ms_ = now;
  int16_t r = ads.readADC_SingleEnded(ADS1115_TEST_CHANNEL);
  last_raw_ = r;
  last_volts_ = ads.computeVolts(r);
  valid_ = true;
  healthy_ = true;
#endif
}

int16_t Ads1115Reader::readRaw(uint8_t channel) {
  if (!begin_ok_ || channel > 3) return 0;
  return ads.readADC_SingleEnded(channel);
}

float Ads1115Reader::readVoltage(uint8_t channel) {
  if (!begin_ok_ || channel > 3) return 0.0f;
  int16_t r = ads.readADC_SingleEnded(channel);
  return ads.computeVolts(r);
}

AdcStats Ads1115Reader::readStats(uint8_t channel, uint8_t n) {
  AdcStats s;
  if (!begin_ok_ || channel > 3 || n == 0) return s;
  if (n > 64) n = 64;
  int32_t sum = 0;
  int16_t mn = 32767, mx = -32768;
  int16_t buf[64];
  for (uint8_t i = 0; i < n; i++) {
    int16_t r = ads.readADC_SingleEnded(channel);
    buf[i] = r;
    sum += r;
    if (r < mn) mn = r;
    if (r > mx) mx = r;
    delay(8);  // ~128 SPS; small gap between conversions (command context only)
  }
  float mean = (float)sum / (float)n;
  float var = 0;
  for (uint8_t i = 0; i < n; i++) {
    float d = (float)buf[i] - mean;
    var += d * d;
  }
  var /= (float)n;
  s.mean = (int16_t)mean;
  s.minv = mn;
  s.maxv = mx;
  s.stddev = sqrtf(var);
  s.volts_mean = ads.computeVolts(s.mean);
  s.n = n;
  s.valid = true;
  return s;
}
