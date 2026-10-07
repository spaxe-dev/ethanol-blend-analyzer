// display.h — OLED abstraction (Phase 1: status screens only, no logic)

#pragma once

#include <Arduino.h>

struct Measurement;  // fwd

class Display {
 public:
  Display();
  bool begin();
  bool isHealthy() const { return healthy_; }
  void showBoot();
  void showSensorStatus(bool temp_ok, bool hx_ok, bool ads_ok, bool oled_ok);
  void showMeasurement(const Measurement& m);
  void showError(const char* line1, const char* line2);
  void showRaw(int32_t raw, bool hx_ok);  // test/hx711-oled: raw readout only

 private:
  void header(const char* title);
  bool healthy_ = false;
};
