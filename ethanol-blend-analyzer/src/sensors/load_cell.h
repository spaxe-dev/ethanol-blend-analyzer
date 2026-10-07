// load_cell.h — HX711 + Y2C131 driver (Phase 1)
//
// VERIFIED WIRING (see config.h): RED->E+, BLACK->E-, GREEN->A-, WHITE->A+.
// A-channel polarity is reversed relative to the naive color mapping;
// GREEN->A+ / WHITE->A- yields garbage raw (0/-1). Do NOT "fix" it back.

#pragma once

#include <Arduino.h>
#include "config.h"

class LoadCell {
 public:
  LoadCell();

  bool begin();
  void update();  // call every loop; throttles internally
  void tare(uint8_t samples = 15);

  int32_t readRaw() const { return last_raw_; }
  float readMassGrams() const { return last_mass_g_; }
  bool isStable() const { return stable_; }
  bool isHealthy() const { return healthy_; }
  bool hasValidMass() const { return valid_; }
  float scaleFactor() const { return scale_factor_; }
  void setScaleFactor(float f) { scale_factor_ = f; applyScale(); }

 private:
  void applyScale();
  void pushHistory(float g);

  int32_t last_raw_ = 0;
  float last_mass_g_ = 0.0f;
  bool valid_ = false;
  bool stable_ = false;
  bool healthy_ = false;
  bool begin_ok_ = false;
  float scale_factor_ = HX711_DEFAULT_SCALE_FACTOR;
  unsigned long last_sample_ms_ = 0;
  unsigned long last_ready_ms_ = 0;
  float history_[HX711_STABLE_WINDOW] = {0};
  uint8_t hist_idx_ = 0;
  uint8_t hist_count_ = 0;
};
