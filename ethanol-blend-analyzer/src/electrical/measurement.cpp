// measurement.cpp — helpers for Measurement (Phase 1: minimal)

#include "electrical/measurement.h"
#include "config.h"

void measurement_begin_unpopulated(Measurement& m) {
  m.timestamp_ms = millis();
  m.temperature_valid = false;
  m.mass_valid = false;
  m.hx711_valid = false;
  m.hx711_stable = false;
  m.volume_valid = false;
  m.density_valid = false;
  m.excitation_frequency_hz = EXCITATION_FREQUENCY_HZ_APPROX;
  m.excitation_frequency_verified = false;
  m.excitation_amplitude = EXCITATION_AMPLITUDE_UNKNOWN;
  m.excitation_amplitude_known = false;
  m.adc_valid = false;
  m.adc_channel = ADS1115_TEST_CHANNEL;
  m.electrical_valid = false;
}
