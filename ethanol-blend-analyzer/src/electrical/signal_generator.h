// signal_generator.h — excitation source abstraction.
// Phase 1 main has NO generator (manual ICL8038 metadata only).
// This branch implements Esp32PwmGenerator: ESP32 LEDC rail-switched
// square wave (0..3.3V) as a stand-in source until the ICL8038 arrives.
// Honest labelling: square wave, source-side amplitude, exact frequency
// (LEDC derives from the crystal, so frequency IS verified).

#pragma once

#include <Arduino.h>
#include "config.h"

class SignalGenerator {
 public:
  virtual ~SignalGenerator() {}
  virtual bool begin() = 0;
  virtual bool setFrequency(float hz) = 0;
  virtual float getFrequency() const = 0;
  virtual void enable() = 0;
  virtual void disable() = 0;
  virtual bool isEnabled() const = 0;
  // Source-side amplitude in volts (pre-divider / pre-cell). The voltage
  // actually reaching the cell depends on the external divider network.
  virtual float sourceAmplitude() const = 0;
};

class Esp32PwmGenerator : public SignalGenerator {
 public:
  Esp32PwmGenerator();
  bool begin() override;
  bool setFrequency(float hz) override;
  float getFrequency() const override { return freq_hz_; }
  void enable() override;
  void disable() override;
  bool isEnabled() const override { return enabled_; }
  float sourceAmplitude() const override { return 3.3f; }

 private:
  float freq_hz_ = SIGGEN_DEFAULT_FREQ_HZ;
  bool enabled_ = false;
  bool begin_ok_ = false;
};
