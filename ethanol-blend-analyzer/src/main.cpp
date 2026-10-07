// main.cpp — HX711 RAW OLED test (test/hx711-oled branch ONLY).
// Shows the live raw HX711 ADC value on OLED + Serial. No grams,
// no set_scale, no calibration. Purpose: observe raw change under force.

#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "display/display.h"
#include "sensors/load_cell.h"

static Display g_display;
static LoadCell g_load;

static bool g_oled_ok = false;

static unsigned long g_last_ui_ms = 0;
static unsigned long g_last_ser_ms = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 1500) {
    delay(10);
  }
  Serial.println(F("# HX711 RAW TEST (no grams, no calibration)"));

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  g_oled_ok = g_display.begin();
  if (g_oled_ok) {
    g_display.showBoot();
  } else {
    Serial.println(F("# OLED INIT FAILED"));
  }

  bool hx_ready = g_load.begin();
  Serial.print(F("# HX711 begin: "));
  Serial.println(hx_ready ? F("READY") : F("NOT READY (check power/wiring)"));
  Serial.println(F("# Apply force to the load cell — raw must change by thousands"));
}

void loop() {
  g_load.update();  // throttled internally to HX711_SAMPLING_INTERVAL_MS

  unsigned long now = millis();

  if (now - g_last_ui_ms >= 300) {
    g_last_ui_ms = now;
    if (g_oled_ok) {
      g_display.showRaw(g_load.readRaw(), g_load.isHealthy());
    }
  }

  if (now - g_last_ser_ms >= 500) {
    g_last_ser_ms = now;
    Serial.print(F("# HX711 RAW "));
    Serial.print(g_load.readRaw());
    Serial.println(g_load.isHealthy() ? F(" READY") : F(" NOT-READY"));
  }
}
