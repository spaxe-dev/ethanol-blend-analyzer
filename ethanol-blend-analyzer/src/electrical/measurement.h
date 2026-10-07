// measurement.h — central measurement data structure (Phase 1)
// Holds real sensor state only. Unavailable fields are flagged invalid,
// never fabricated.

#pragma once

#include <Arduino.h>
#include "config.h"

struct Measurement {
  uint32_t timestamp_ms = 0;

  float temperature_c = 0.0f;
  bool temperature_valid = false;

  float mass_g = 0.0f;
  bool mass_valid = false;
  int32_t hx711_raw = 0;
  bool hx711_valid = false;
  bool hx711_stable = false;

  // Phase 1: no sample vessels yet -> always invalid.
  float volume_ml = 0.0f;
  bool volume_valid = false;
  float density_g_ml = 0.0f;
  bool density_valid = false;

  // Excitation metadata only (ICL8038 is manual). Not a live measurement.
  float excitation_frequency_hz = EXCITATION_FREQUENCY_HZ_APPROX;
  bool excitation_frequency_verified = false;  // always false in Phase 1
  float excitation_amplitude = EXCITATION_AMPLITUDE_UNKNOWN;
  bool excitation_amplitude_known = false;

  // ADS1115 Phase-1 test channel state (safe known-voltage test, NOT fuel).
  int16_t adc_raw = 0;
  float adc_voltage = 0.0f;
  uint8_t adc_channel = ADS1115_TEST_CHANNEL;
  bool adc_valid = false;

  // Electrical fuel response: NOT IMPLEMENTED in Phase 1. Always invalid.
  float electrical_response = 0.0f;
  bool electrical_valid = false;
};

void measurement_begin_unpopulated(Measurement& m);
