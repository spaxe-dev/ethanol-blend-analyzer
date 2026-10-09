// config.h — EthanolBlend centralized hardware configuration (Phase 1)
// ALL GPIO / I2C / calibration knobs live here. No other file may hard-code pins.
//
// BOARD ASSUMPTION: generic ESP32 DOIT DEVKIT V1 (30-pin).
// If your board differs, only change this file.
//
// WIRING ASSUMPTIONS (must match physical wiring, see handoff report):
//   OLED SDA -> GPIO21, SCL -> GPIO22 (shared I2C bus with ADS1115)
//   DS18B20 DQ -> GPIO4 (requires external 4.7k pull-up to 3.3V)
//   HX711 DOUT -> GPIO16, SCK -> GPIO17
//   ADS1115 shares I2C bus (SDA 21 / SCL 22), ADDR pin -> GND = 0x48
// If you wire differently, update the defines below — nothing else changes.

#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>  // for gain type in config (header-only type)

// ---------- Serial ----------
#define SERIAL_BAUD 115200

// ---------- Feature flags (Phase 1) ----------
// Electrical fuel analysis is NOT implemented in Phase 1.
// ICL8038 / LM358 / electrode cell are intentionally absent.
#define OLED_ENABLED true
#define DS18B20_ENABLED true
#define HX711_ENABLED true
#define ADS1115_ENABLED true

// ---------- I2C ----------
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_CLOCK_HZ 400000

// ---------- OLED ----------
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH_PX 128
#define OLED_HEIGHT_PX 64
#define OLED_RESET_PIN -1  // most 0.96" modules have no reset pin

// ---------- DS18B20 ----------
#define DS18B20_PIN 4
#define DS18B20_RESOLUTION_BITS 12
// Invalid-temperature sentinel handling is in the driver; never use 0 as "valid".
#define TEMP_INVALID_C -127.0f
#define TEMP_SAMPLING_INTERVAL_MS 2000

// ---------- HX711 / Y2C131 ----------
#define HX711_DOUT_PIN 16
#define HX711_SCK_PIN 17
#define HX711_SAMPLING_INTERVAL_MS 250
// !!! UNCALIBRATED placeholder. Must be set with `set_scale <v>` + known mass.
// Do NOT treat readings in grams as accurate until calibrated.
#define HX711_DEFAULT_SCALE_FACTOR 1000.0f
#define HX711_STABLE_WINDOW 10
#define HX711_STABLE_TOL_G 1.0f

// ---------- ADS1115 (Phase 1: comms + safe-voltage test only) ----------
// Power the ADS1115 module from 3.3V (VDD=3.3V) so SDA/SCL stay at 3.3V logic.
// Absolute input limit is then ~GND-0.3V .. VDD+0.3V (~3.6V max). NEVER feed 5V.
#define ADS1115_I2C_ADDRESS 0x48  // ADDR pin tied to GND
#define ADS1115_GAIN GAIN_ONE     // +/-4.096V full-scale; LSB = 125uV. Safe default.
// 860 SPS on feat/esp32-excitation: resolves <=100 Hz excitation cycles
// (43 samples/cycle at the 20 Hz default) for min/max capture.
#define ADS1115_DATARATE RATE_ADS1115_860SPS
#define ADS1115_SAMPLING_INTERVAL_MS 500
#define ADS1115_TEST_CHANNEL 0     // Phase-1 test channel (A0 vs GND)
#define ADS1115_AVG_SAMPLES 16    // averaging for stats

// ---------- Measurement / UI timing ----------
#define DISPLAY_REFRESH_INTERVAL_MS 1000
#define MEASUREMENT_PUBLISH_INTERVAL_MS 2000

// ---------- Sample metadata (Phase 1: NO fuel, so volume/density invalid) ----------
// Volume is unknown until proper sample vessels exist. Leave invalid.
#define SAMPLE_VOLUME_ML_INVALID true

// ---------- Excitation metadata (Phase 1: NOT measured, manual generator only) ----------
// ICL8038 is manually adjusted. This value is CONFIG/METADATA ONLY —
// it is NOT verified by firmware in Phase 1. Clearly marked approximate.
#define EXCITATION_FREQUENCY_HZ_APPROX 1000.0f
// Amplitude unknown until analog front-end is characterized. -1 = unknown.
#define EXCITATION_AMPLITUDE_UNKNOWN -1.0f

// ---------- Signal generator (feat/esp32-excitation: ESP32 stand-in source) ----------
// ESP32 LEDC square wave on a free GPIO until the ICL8038 module arrives.
// Chosen so the ADS1115 (860 SPS here) fully resolves each cycle at default.
// Square 0..3.3V rail; cell-side amplitude depends on the external divider.
#define SIGGEN_ENABLED true
#define SIGGEN_PIN 26             // free, non-strapping; DAC-capable for later sine work
#define SIGGEN_LEDC_CHANNEL 0
// 14-bit: LEDC divider stays in hardware range (<=1023) for 5..1000 Hz.
// (10-bit cannot make 20 Hz: divider 3906 exceeds the max 1023.)
#define SIGGEN_LEDC_RES_BITS 14   // 50% duty = 8192
#define SIGGEN_DEFAULT_FREQ_HZ 20.0f
#define SIGGEN_MIN_FREQ_HZ 5.0f
#define SIGGEN_MAX_FREQ_HZ 1000.0f
