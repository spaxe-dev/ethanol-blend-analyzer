// temperature.cpp — DS18B20 via OneWire + DallasTemperature (Phase 1)
// Invalid readings (DEVICE_DISCONNECTED_C) are flagged, never coerced to 0.

#include "sensors/temperature.h"
#include "config.h"

#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(DS18B20_PIN);
static DallasTemperature dallas(&oneWire);

TemperatureSensor::TemperatureSensor() {}

bool TemperatureSensor::begin() {
#if !DS18B20_ENABLED
  healthy_ = false;
  return false;
#else
  dallas.begin();
  dallas.setResolution(DS18B20_RESOLUTION_BITS);
  dallas.setWaitForConversion(true);  // simple blocking read inside update interval
  sensor_count_ = dallas.getDeviceCount();
  begin_ok_ = true;
  healthy_ = (sensor_count_ > 0);
  last_sample_ms_ = 0;  // force immediate first sample in update()
  return healthy_;
#endif
}

void TemperatureSensor::update() {
#if !DS18B20_ENABLED
  return;
#else
  if (!begin_ok_) return;
  unsigned long now = millis();
  if (now - last_sample_ms_ < TEMP_SAMPLING_INTERVAL_MS && last_sample_ms_ != 0) {
    return;
  }
  last_sample_ms_ = now;

  dallas.requestTemperatures();
  float t = dallas.getTempCByIndex(0);

  // Dallas returns DEVICE_DISCONNECTED_C (-127) on failure.
  if (t == DEVICE_DISCONNECTED_C || t < -100.0f || t > 125.0f) {
    valid_ = false;
    healthy_ = (sensor_count_ > 0);  // bus present but read failed this cycle
    return;
  }
  last_temp_c_ = t;
  valid_ = true;
  healthy_ = true;
#endif
}
