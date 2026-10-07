// display.cpp — 0.96" 128x64 I2C OLED via Adafruit SSD1306 (Phase 1)

#include "display/display.h"
#include "config.h"
#include "electrical/measurement.h"

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

// test/hx711-oled: raw HX711 readout. No grams, no calibration.
void Display::showRaw(int32_t raw, bool hx_ok) {
  if (!healthy_) return;
  header("HX711 RAW");
  oled.setTextSize(2);
  oled.setCursor(0, 30);
  oled.println(raw);
  oled.setTextSize(1);
  oled.print(F("HX711: "));
  oled.println(hx_ok ? F("OK") : F("ERROR"));
  oled.display();
}
