// lookup.cpp — interpolation over measured reference rows.

#include "calibration/lookup.h"

// Measured max values, descending (higher ethanol => lower response).
static const LookupRow TABLE[] = {
  { 26432, 0.0f },    // E0  base market petrol
  { 25936, 9.0f },    // E9  +1 ml
  { 25694, 17.0f },   // E17 +2 ml
  { 25325, 23.0f },   // E23 +3 ml
  { 24950, 29.0f },   // E29 +4 ml (avg of 24909/24991, same cup lineage)
  { 24762, 33.0f },   // E33 +5 ml (same cup lineage)
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

void modelFit(LinFit& f) {
  // Ordinary least squares: pct = slope * max + intercept.
  double sx = 0, sy = 0, sxx = 0, sxy = 0;
  for (uint8_t i = 0; i < NROWS; i++) {
    double x = TABLE[i].adc_max, y = TABLE[i].added_pct;
    sx += x;
    sy += y;
    sxx += x * x;
    sxy += x * y;
  }
  double denom = (double)NROWS * sxx - sx * sx;
  if (denom == 0.0) {
    f.fitted = false;
    return;
  }
  f.slope = (float)(((double)NROWS * sxy - sx * sy) / denom);
  f.intercept = (float)((sy - (double)f.slope * sx) / (double)NROWS);
  f.max_residual = 0.0f;
  for (uint8_t i = 0; i < NROWS; i++) {
    float r = fabsf(f.slope * TABLE[i].adc_max + f.intercept - TABLE[i].added_pct);
    if (r > f.max_residual) f.max_residual = r;
  }
  f.fitted = true;
}

float regressAddedEthanol(int16_t adc_max, const LinFit& f) {
  if (!f.fitted || adc_max < 1000) return NAN;
  return f.slope * adc_max + f.intercept;
}

void bucketLabel(float added_pct, char* buf, size_t bufsz) {
  // Reference blends we actually tabled; tolerance = claimed +/-5 pts.
  static const float REFS[] = { 0.0f, 9.0f, 17.0f, 23.0f, 29.0f, 33.0f };
  static const uint8_t NREFS = sizeof(REFS) / sizeof(REFS[0]);
  if (isnan(added_pct)) {
    snprintf(buf, bufsz, "NO READING");
    return;
  }
  uint8_t best = 0;
  for (uint8_t i = 1; i < NREFS; i++) {
    if (fabsf(added_pct - REFS[i]) < fabsf(added_pct - REFS[best])) best = i;
  }
  if (fabsf(added_pct - REFS[best]) <= 5.0f) {
    snprintf(buf, bufsz, "LIKELY E%d", (int)(REFS[best] + 0.5f));
    return;
  }
  // Between references: name the bracket low-high.
  uint8_t lo = best, hi = best;
  if (added_pct < REFS[best] && best > 0) {
    lo = best - 1;
    hi = best;
  } else if (added_pct > REFS[best] && best + 1 < NREFS) {
    lo = best;
    hi = best + 1;
  }
  if (lo == hi) {
    snprintf(buf, bufsz, added_pct < REFS[0] ? "BELOW E0" : "ABOVE E33");
  } else {
    snprintf(buf, bufsz, "E%d-E%d", (int)(REFS[lo] + 0.5f), (int)(REFS[hi] + 0.5f));
  }
}
