// temperature.h — DS18B20 driver (Phase 1)

#pragma once

#include <Arduino.h>
#include "config.h"

class TemperatureSensor {
 public:
  TemperatureSensor();

  // Returns true if at least one sensor responded on the OneWire bus.
  bool begin();
  // Non-blocking: call every loop; internally throttles to sampling interval.
  void update();
  // Last cached reading. Check isHealthy()/isValid() first.
  float readTemperature() const { return last_temp_c_; }
  bool isValid() const { return valid_; }
  bool isHealthy() const { return healthy_; }
  uint8_t sensorCount() const { return sensor_count_; }

 private:
  float last_temp_c_ = TEMP_INVALID_C;
  bool valid_ = false;
  bool healthy_ = false;
  uint8_t sensor_count_ = 0;
  unsigned long last_sample_ms_ = 0;
  bool begin_ok_ = false;
};
