// lookup.cpp — interpolation over measured reference rows.

#include "calibration/lookup.h"

// Measured max values, descending (higher ethanol => lower response).
static const LookupRow TABLE[] = {
  { 26432, 0.0f },    // E0  base market petrol
  { 25936, 9.0f },    // E9  +1 ml
  { 25694, 17.0f },   // E17 +2 ml
  { 25325, 23.0f },   // E23 +3 ml
};
static const uint8_t NROWS = sizeof(TABLE) / sizeof(TABLE[0]);

uint8_t lookupRowCount() { return NROWS; }

float lookupAddedEthanol(int16_t adc_max) {
  if (adc_max < 1000) return NAN;  // out of family: open leads / fault
  if (adc_max >= TABLE[0].adc_max) return TABLE[0].added_pct;
  if (adc_max <= TABLE[NROWS - 1].adc_max) return TABLE[NROWS - 1].added_pct;
  for (uint8_t i = 0; i < NROWS - 1; i++) {
    if (adc_max <= TABLE[i].adc_max && adc_max >= TABLE[i + 1].adc_max) {
      float span = (float)(TABLE[i].adc_max - TABLE[i + 1].adc_max);
      float frac = (float)(TABLE[i].adc_max - adc_max) / span;
      return TABLE[i].added_pct +
             frac * (TABLE[i + 1].added_pct - TABLE[i].added_pct);
    }
  }
  return NAN;  // unreachable
}
