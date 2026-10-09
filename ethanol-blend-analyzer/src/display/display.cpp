// display.cpp — 0.96" 128x64 I2C OLED via Adafruit SSD1306 (Phase 1)

#include "display/display.h"
#include "config.h"
#include "electrical/measurement.h"
#include "calibration/lookup.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 oled(OLED_WIDTH_PX, OLED_HEIGHT_PX, &Wire, OLED_RESET_PIN);

Display::Display() {}

bool Display::begin() {
#if !OLED_ENABLED
  healthy_ = false;
  return false;
#else
  // Wire.begin() already done in main.cpp with configured pins.
  bool ok = oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  healthy_ = ok;
  if (ok) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
  }
  return ok;
#endif
}

void Display::header(const char* title) {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println(F("ETHANOLBLEND"));
  oled.println(title);
  oled.println(F("---------------------"));
}

void Display::showBoot() {
  if (!healthy_) return;
  header("SYSTEM BOOT");
  oled.println(F("Phase 1 bring-up"));
  oled.println(F("OLED OK"));
  oled.display();
}

void Display::showSensorStatus(bool temp_ok, bool hx_ok, bool ads_ok, bool oled_ok) {
  if (!healthy_) return;
  header("SYSTEM CHECK");
  oled.print(F("OLED: ")); oled.println(oled_ok ? F("OK") : F("FAIL"));
  oled.print(F("TEMP: ")); oled.println(temp_ok ? F("OK") : F("FAIL"));
  oled.print(F("HX711: ")); oled.println(hx_ok ? F("OK") : F("FAIL"));
  oled.print(F("ADS1115: ")); oled.println(ads_ok ? F("OK") : F("FAIL"));
  oled.display();
}

void Display::showMeasurement(const Measurement& m) {
  if (!healthy_) return;
  header("READY  (PH1)");
  oled.print(F("TEMP: "));
  if (m.temperature_valid) {
    oled.print(m.temperature_c, 1);
    oled.println(F(" C"));
  } else {
    oled.println(F("--.- C"));
  }
  oled.print(F("MASS: "));
  if (m.mass_valid) {
    oled.print(m.mass_g, 1);
    oled.println(F(" g"));
  } else {
    oled.println(F("--.-- g"));
  }
  oled.print(F("ADC: "));
  if (m.adc_valid) {
    oled.print(m.adc_voltage, 3);
    oled.println(F(" V"));
  } else {
    oled.println(F("FAIL"));
  }
  oled.display();
}

void Display::showError(const char* line1, const char* line2) {
  if (!healthy_) return;
  header("ERROR");
  oled.println(line1);
  oled.println(line2);
  oled.display();
}

// Estimate from the measured lookup table (added-% over market-petrol base).
// Stage layout: inverted title bar, big bucket, ethanol gauge bar, data line.
void Display::showEstimate(float added_pct, bool valid, float resp_v,
                           float temp_c, bool temp_valid) {
  if (!healthy_) return;
  char bucket[12];
  if (valid) {
    bucketLabel(added_pct, bucket, sizeof(bucket));
  } else {
    snprintf(bucket, sizeof(bucket), "NO READING");
  }
  oled.clearDisplay();
  // Inverted title bar.
  oled.fillRect(0, 0, OLED_WIDTH_PX, 9, SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_BLACK);
  oled.setCursor(22, 1);
  oled.print(F("ETHANOLBLEND"));
  oled.setTextColor(SSD1306_WHITE);
  // Bucket, big.
  oled.setTextSize(2);
  oled.setCursor(0, 12);
  oled.println(bucket);
  // Gauge bar 0..40% (covers table + margin).
  float frac = valid ? added_pct / 40.0f : 0.0f;
  if (frac < 0) frac = 0;
  if (frac > 1) frac = 1;
  oled.drawRect(0, 32, OLED_WIDTH_PX, 10, SSD1306_WHITE);
  oled.fillRect(2, 34, (int)(frac * (OLED_WIDTH_PX - 4)), 6, SSD1306_WHITE);
  // Data + cal lines.
  oled.setTextSize(1);
  oled.setCursor(0, 44);
  oled.print(F("+"));
  if (valid) {
    oled.print(added_pct, 1);
  } else {
    oled.print(F("--.-"));
  }
  oled.print(F("% R"));
  oled.print(resp_v, 2);
  oled.print(F(" T"));
  if (temp_valid) {
    oled.print(temp_c, 0);
  } else {
    oled.print(F("--"));
  }
  oled.setCursor(0, 54);
  oled.print(F("CAL:6PT BASE=MKT"));
  oled.display();
}

void Display::showBusy() {
  if (!healthy_) return;
  uint8_t dots = (millis() / 400) % 4;  // 0..3 cycling pips
  oled.clearDisplay();
  oled.drawRect(0, 16, OLED_WIDTH_PX, 32, SSD1306_WHITE);
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 21);
  oled.print(F("MEASURING"));
  for (uint8_t i = 0; i < dots; i++) {
    oled.fillRect(52 + i * 9, 39, 6, 5, SSD1306_WHITE);
  }
  oled.display();
}
