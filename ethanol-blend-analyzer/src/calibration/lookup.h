// lookup.h — reference-table ethanol estimation (real measured data only).
//
// PROVENANCE (bench, night run, feat/esp32-excitation):
//   10 ml market petrol ("supposedly petrol", likely already oxygenated —
//   treated as base 0) + 0 / 1 / 2 / 3 ml ethanol => added fractions
//   E0 / E9 / E17 / E23. Same cup, same stick, same depth, wiped leads.
//   Excitation: ESP32 20 Hz square via 10 k series. ADS1115 860 SPS,
//   stats over 64 samples; table keyed on max (high-phase response).
//   temp ~28-29 C throughout. Rows below are those measured max values.
// NEVER append invented rows. New blends get measured before tabling.

#pragma once

#include <Arduino.h>

struct LookupRow {
  int16_t adc_max;      // measured high-phase response (counts)
  float added_pct;      // measured added-ethanol percent for that row
};

// Piecewise-linear interpolation over the table. Clamps outside range.
// Returns NaN when adc_max is clearly out of family (<1000 counts).
float lookupAddedEthanol(int16_t adc_max);

// Number of rows (report as calibration basis, e.g. "4PT").
uint8_t lookupRowCount();
