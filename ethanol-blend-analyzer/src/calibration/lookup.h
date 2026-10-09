// lookup.h — reference-table ethanol estimation (real measured data only).
//
// PROVENANCE (bench, night run, feat/esp32-excitation):
//   10 ml market petrol ("supposedly petrol", likely already oxygenated —
//   treated as base 0) + 0 / 1 / 2 / 3 ml ethanol => added fractions
//   E0 / E9 / E17 / E23. Extended same night, same cup lineage:
//   E29 (+4 ml, avg 24909/24991) / E33 (+5 ml, 24762).
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

// Least-squares linear fit over the SAME rows (single source of truth).
// pct = slope * adc_max + intercept. Fit once via modelFit(), then predict.
// The fit smooths jig noise but must NOT be trusted far outside the rows;
// modelResidualMax() reports the worst row error as the honesty metric.
struct LinFit {
  float slope = 0.0f;
  float intercept = 0.0f;
  float max_residual = 0.0f;  // worst |predicted - row| in added-%
  bool fitted = false;
};

void modelFit(LinFit& f);
float regressAddedEthanol(int16_t adc_max, const LinFit& f);

// Coarse bucket label for display: nearest reference row within tolerance,
// else the bracketing range. Never invents precision the data can't hold.
// Writes e.g. "LIKELY E20" or "E10-E20" into buf (bufsz >= 12).
void bucketLabel(float added_pct, char* buf, size_t bufsz);
