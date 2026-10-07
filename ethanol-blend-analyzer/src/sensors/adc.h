// adc.h — ADS1115 abstraction (Phase 1: comms + safe-voltage test)
// Hardware-neutral API so later phases don't care which ADC is selected.
// Phase 1 implements ONLY the ADS1115 path (the confirmed hardware).

#pragma once

#include <Arduino.h>

// Forward-declared so callers don't need the Adafruit header.
struct AdcStats {
  int16_t mean = 0;
  int16_t minv = 0;
  int16_t maxv = 0;
  float stddev = 0.0f;
  float volts_mean = 0.0f;
  uint8_t n = 0;
  bool valid = false;
};

class AnalogReader {
 public:
  virtual ~AnalogReader() {}
  virtual bool begin() = 0;
  virtual int16_t readRaw(uint8_t channel) = 0;
  virtual float readVoltage(uint8_t channel) = 0;
  virtual bool isHealthy() = 0;
};

class Ads1115Reader : public AnalogReader {
 public:
  Ads1115Reader();
  bool begin() override;
  void update();  // background sample of test channel
  int16_t readRaw(uint8_t channel) override;
  float readVoltage(uint8_t channel) override;
  bool isHealthy() override { return healthy_; }

  // Last background sample (test channel) + validity.
  int16_t lastRaw() const { return last_raw_; }
  float lastVolts() const { return last_volts_; }
  bool hasValid() const { return valid_; }

  // Averaging helper for Phase-2 noise characterization.
  AdcStats readStats(uint8_t channel, uint8_t n);

 private:
  bool healthy_ = false;
  bool begin_ok_ = false;
  int16_t last_raw_ = 0;
  float last_volts_ = 0.0f;
  bool valid_ = false;
  unsigned long last_sample_ms_ = 0;
};
