// load_cell.cpp — HX711 (bogde) + Y2C131 1kg (Phase 1)
// Raw readings always available; grams are UNCALIBRATED until user runs
// `set_scale` with a known mass. Never present grams as accurate before that.

#include "sensors/load_cell.h"
#include "config.h"

#include "HX711.h"

static HX711 hx711;

LoadCell::LoadCell() {}

void LoadCell::applyScale() {
  if (begin_ok_) {
    hx711.set_scale(scale_factor_);
  }
}

bool LoadCell::begin() {
#if !HX711_ENABLED
  healthy_ = false;
  return false;
#else
  hx711.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  hx711.set_scale(scale_factor_);
  // Do NOT auto-tare to a fake zero claim: attempt tare but tolerate no load.
  // If chip not present, is_ready() will stay false and healthy_=false.
  begin_ok_ = true;
  last_sample_ms_ = 0;
  // Best-effort tare; harmless if no load.
  if (hx711.wait_ready_timeout(1000)) {
    hx711.tare(10);
    last_ready_ms_ = millis();
    healthy_ = true;
  } else {
    healthy_ = false;
  }
  return healthy_;
#endif
}

void LoadCell::tare(uint8_t samples) {
  if (!begin_ok_) return;
  if (hx711.wait_ready_timeout(1000)) {
    hx711.tare(samples);
    hist_count_ = 0;
    hist_idx_ = 0;
  }
}

void LoadCell::pushHistory(float g) {
  history_[hist_idx_] = g;
  hist_idx_ = (hist_idx_ + 1) % HX711_STABLE_WINDOW;
  if (hist_count_ < HX711_STABLE_WINDOW) hist_count_++;
}

void LoadCell::update() {
#if !HX711_ENABLED
  return;
#else
  if (!begin_ok_) return;
  unsigned long now = millis();
  if (now - last_sample_ms_ < HX711_SAMPLING_INTERVAL_MS && last_sample_ms_ != 0) {
    return;
  }
  last_sample_ms_ = now;

  if (!hx711.is_ready()) {
    // No fresh data. If silent for >2s, mark unhealthy (wiring/power issue).
    if (now - last_ready_ms_ > 2000) {
      healthy_ = false;
      valid_ = false;
      stable_ = false;
    }
    return;
  }
  last_ready_ms_ = now;
  healthy_ = true;

  // read_average gives raw-ish average; get_units applies scale.
  // Use 1 sample here (we already throttle); averaging for stability via history.
  long raw = hx711.read_average(1);
  float grams = hx711.get_units(1);

  last_raw_ = (int32_t)raw;
  // Reject obvious garbage: HX711 24-bit range check.
  if (raw == 0x7FFFFF || raw == (long)0xFF800000) {
    valid_ = false;
    stable_ = false;
    return;
  }
  last_mass_g_ = grams;
  valid_ = true;

  pushHistory(grams);
  if (hist_count_ >= HX711_STABLE_WINDOW) {
    float mn = history_[0], mx = history_[0];
    for (uint8_t i = 1; i < HX711_STABLE_WINDOW; i++) {
      if (history_[i] < mn) mn = history_[i];
      if (history_[i] > mx) mx = history_[i];
    }
    stable_ = ((mx - mn) <= HX711_STABLE_TOL_G);
  } else {
    stable_ = false;
  }
#endif
}
