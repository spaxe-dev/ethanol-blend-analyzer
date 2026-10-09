// main.cpp — EthanolBlend Phase 1 bring-up
// Phase 1 scope ONLY: boot, serial, OLED, DS18B20, HX711/Y2C131, ADS1115 test,
// measurement struct, JSON Lines output. No fuel logic, no ICL8038 drive,
// no impedance claims, no calibration model.

#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "electrical/measurement.h"
#include "electrical/signal_generator.h"
#include "sensors/temperature.h"
#include "sensors/load_cell.h"
#include "sensors/adc.h"
#include "display/display.h"
#include "communication/serial_protocol.h"

static TemperatureSensor g_temp;
static LoadCell g_load;
static Ads1115Reader g_adc;
static Esp32PwmGenerator g_sig;
static Display g_display;
static SerialProtocol g_serial;
static Measurement g_meas;

static bool g_oled_ok = false;
static bool g_temp_ok = false;
static bool g_hx_ok = false;
static bool g_ads_ok = false;

static unsigned long g_last_display_ms = 0;
static unsigned long g_last_publish_ms = 0;
static unsigned long g_last_status_ms = 0;
static bool g_status_shown = false;

static void refreshMeasurement() {
  g_meas.timestamp_ms = millis();

  g_meas.temperature_c = g_temp.readTemperature();
  g_meas.temperature_valid = g_temp.isValid();

  g_meas.mass_g = g_load.readMassGrams();
  g_meas.mass_valid = g_load.hasValidMass();
  g_meas.hx711_raw = g_load.readRaw();
  g_meas.hx711_valid = g_load.isHealthy() && g_load.hasValidMass();
  g_meas.hx711_stable = g_load.isStable();

  g_meas.volume_valid = false;   // no vessels in Phase 1
  g_meas.density_valid = false;  // cannot compute without volume

  g_meas.excitation_frequency_hz = g_sig.getFrequency();
  g_meas.excitation_frequency_verified = true;  // LEDC derives from crystal
  g_meas.excitation_amplitude = g_sig.isEnabled() ? g_sig.sourceAmplitude() : 0.0f;
  g_meas.excitation_amplitude_known = true;  // source-side, pre-divider square

  g_meas.adc_channel = ADS1115_TEST_CHANNEL;
  g_meas.adc_raw = g_adc.lastRaw();
  g_meas.adc_voltage = g_adc.lastVolts();
  g_meas.adc_valid = g_adc.hasValid();

  g_meas.electrical_valid = false;  // NOT IMPLEMENTED in Phase 1
}

void setup() {
  g_serial.begin(SERIAL_BAUD);
  g_serial.printBootBanner();

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);

  // Order: display first (so errors are visible), then sensors.
  g_oled_ok = g_display.begin();
  if (g_oled_ok) {
    g_display.showBoot();
  }

  g_temp_ok = g_temp.begin();
  g_hx_ok = g_load.begin();
  g_ads_ok = g_adc.begin();
  bool sig_ok = g_sig.begin();
  if (sig_ok) {
    Serial.print(F("# SIG on: ESP32 LEDC square "));
    Serial.print(g_sig.getFrequency(), 1);
    Serial.println(F(" Hz 0..3.3V on GPIO26 (stand-in until ICL8038)"));
  } else {
    Serial.println(F("# SIG generator disabled in this build"));
  }

  g_serial.printStatus(g_temp.isHealthy(), g_load.isHealthy(), g_adc.isHealthy(), g_oled_ok);
  Serial.println(F("# HX711 grams UNCALIBRATED until set_scale with known mass"));
  Serial.println(F("# ADS1115: power module from 3.3V; keep inputs within 0..3.3V"));

  if (g_oled_ok) {
    // First paint uses live health flags (temp/hx may still be sampling).
    g_display.showSensorStatus(g_temp.isHealthy(), g_load.isHealthy(),
                              g_adc.isHealthy(), g_oled_ok);
  }

  measurement_begin_unpopulated(g_meas);
}

void loop() {
  unsigned long now = millis();

  // 1. Serial commands (always responsive).
  g_serial.handleCommands(g_temp, g_load, g_adc, &g_sig);

  // 2. Sensor updates (each throttled internally, non-blocking).
  g_temp.update();
  g_load.update();
  g_adc.update();

  refreshMeasurement();

  // 3. Display refresh at 1 Hz.
  if (now - g_last_display_ms >= DISPLAY_REFRESH_INTERVAL_MS) {
    g_last_display_ms = now;
    if (g_oled_ok) {
      // Re-show system check once sensors settle, then live measurements.
      if (!g_status_shown && now > 3000) {
        g_display.showSensorStatus(g_temp.isHealthy(), g_load.isHealthy(),
                                   g_adc.isHealthy(), g_oled_ok);
        g_status_shown = true;
      } else if (g_status_shown) {
        g_display.showMeasurement(g_meas);
      }
    }
  }

  // 4. Machine-readable measurement publish at 0.5 Hz.
  if (now - g_last_publish_ms >= MEASUREMENT_PUBLISH_INTERVAL_MS) {
    g_last_publish_ms = now;
    g_serial.publishMeasurement(g_meas);
  }

  // 5. Periodic human-readable health reminder (every 10 s, '#' prefixed).
  if (now - g_last_status_ms >= 10000) {
    g_last_status_ms = now;
    g_serial.printStatus(g_temp.isHealthy(), g_load.isHealthy(),
                         g_adc.isHealthy(), g_oled_ok);
  }
}
