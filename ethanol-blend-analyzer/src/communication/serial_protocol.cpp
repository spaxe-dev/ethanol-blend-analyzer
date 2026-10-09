// serial_protocol.cpp — Phase 1 serial commands + JSON Lines
// Convention: lines starting with '#' are human debug; lines starting
// with '{' are machine JSON. PC parsers should ignore non-'{' lines.

#include "communication/serial_protocol.h"
#include "config.h"
#include "electrical/measurement.h"
#include "electrical/signal_generator.h"
#include "calibration/lookup.h"
#include "sensors/temperature.h"
#include "sensors/load_cell.h"
#include "sensors/adc.h"

void SerialProtocol::begin(unsigned long baud) {
  Serial.begin(baud);
  // Wait briefly for USB serial on dev boards (non-blocking fallback).
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 1500) {
    delay(10);
  }
}

void SerialProtocol::printBootBanner() {
  Serial.println(F("# ETHANOLBLEND Phase 1 bring-up"));
  Serial.println(F("# type 'help' for commands; JSON lines start with '{'"));
}

void SerialProtocol::printStatus(bool temp_ok, bool hx_ok, bool ads_ok, bool oled_ok) {
  Serial.print(F("# STATUS oled="));
  Serial.print(oled_ok ? F("OK") : F("FAIL"));
  Serial.print(F(" temp="));
  Serial.print(temp_ok ? F("OK") : F("FAIL"));
  Serial.print(F(" hx711="));
  Serial.print(hx_ok ? F("OK") : F("FAIL"));
  Serial.print(F(" ads1115="));
  Serial.println(ads_ok ? F("OK") : F("FAIL"));
}

void SerialProtocol::printHelp() {
  Serial.println(F("# commands:"));
  Serial.println(F("#   help            - this text"));
  Serial.println(F("#   status          - sensor health"));
  Serial.println(F("#   temp            - read temperature once"));
  Serial.println(F("#   hx711_raw       - raw HX711 average"));
  Serial.println(F("#   weight          - mass in grams (UNCALIBRATED until set_scale)"));
  Serial.println(F("#   tare            - tare load cell"));
  Serial.println(F("#   set_scale <f>   - set HX711 scale factor (needs known mass)"));
  Serial.println(F("#   adc [ch]        - read ADS1115 channel 0-3 (default 0)"));
  Serial.println(F("#   adc_stats [ch] [n] - mean/min/max/stddev over n samples"));
  Serial.println(F("#   sig [hz|on|off] - excitation state / set freq / enable / disable"));
  Serial.println(F("#   sig blink     - 6x slow toggle on GPIO26 (find pin w/ meter)"));
  Serial.println(F("#   predict       - fresh estimate from lookup table"));
}

String SerialProtocol::readLine() {
  String s = Serial.readStringUntil('\n');
  s.trim();
  return s;
}

static void printJsonFloatOrNull(float v, bool valid, int decimals) {
  if (!valid || isnan(v)) {
    Serial.print(F("null"));
  } else {
    Serial.print(v, decimals);
  }
}

void SerialProtocol::publishMeasurement(const Measurement& m) {
  // One JSON object per line. Unavailable fields are null — never fabricated.
  Serial.print(F("{\"type\":\"measurement\""));
  Serial.print(F(",\"timestamp_ms\":"));
  Serial.print(m.timestamp_ms);
  Serial.print(F(",\"temperature_c\":"));
  printJsonFloatOrNull(m.temperature_c, m.temperature_valid, 2);
  Serial.print(F(",\"mass_g\":"));
  printJsonFloatOrNull(m.mass_g, m.mass_valid, 2);
  Serial.print(F(",\"hx711_raw\":"));
  if (m.hx711_valid) {
    Serial.print(m.hx711_raw);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(",\"hx711_stable\":"));
  Serial.print(m.hx711_stable ? F("true") : F("false"));
  Serial.print(F(",\"volume_ml\":null,\"density_g_ml\":null"));
  Serial.print(F(",\"excitation_frequency_hz\":"));
  Serial.print(m.excitation_frequency_hz, 1);
  Serial.print(F(",\"excitation_frequency_verified\":"));
  Serial.print(m.excitation_frequency_verified ? F("true") : F("false"));
  Serial.print(F(",\"adc_channel\":"));
  Serial.print(m.adc_channel);
  Serial.print(F(",\"adc_raw\":"));
  if (m.adc_valid) {
    Serial.print(m.adc_raw);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(",\"adc_voltage\":"));
  printJsonFloatOrNull(m.adc_voltage, m.adc_valid, 4);
  Serial.print(F(",\"electrical_response\":"));
  printJsonFloatOrNull(m.electrical_response, m.electrical_valid, 4);
  Serial.println(F("}"));
}

void SerialProtocol::handleCommands(TemperatureSensor& temp, LoadCell& load, Ads1115Reader& adc,
                               SignalGenerator* sig) {
  if (!Serial.available()) return;
  String line = readLine();
  if (line.length() == 0) return;

  String cmd = line;
  String arg;
  int sp = line.indexOf(' ');
  if (sp >= 0) {
    cmd = line.substring(0, sp);
    arg = line.substring(sp + 1);
    cmd.trim();
    arg.trim();
  }
  cmd.toLowerCase();

  if (cmd == "help") {
    printHelp();
  } else if (cmd == "status") {
    Serial.print(F("# temp="));
    Serial.print(temp.isHealthy() ? F("OK") : F("FAIL"));
    Serial.print(F(" valid="));
    Serial.print(temp.isValid() ? F("1") : F("0"));
    Serial.print(F(" hx711="));
    Serial.print(load.isHealthy() ? F("OK") : F("FAIL"));
    Serial.print(F(" ads1115="));
    Serial.println(adc.isHealthy() ? F("OK") : F("FAIL"));
  } else if (cmd == "temp") {
    if (temp.isValid()) {
      Serial.print(F("# temp_c="));
      Serial.println(temp.readTemperature(), 2);
    } else {
      Serial.println(F("# temp INVALID (sensor missing or read failed)"));
    }
  } else if (cmd == "tare") {
    load.tare();
    Serial.println(F("# tared"));
  } else if (cmd == "hx711_raw") {
    Serial.print(F("# hx711_raw="));
    Serial.println(load.readRaw());
  } else if (cmd == "weight") {
    if (load.hasValidMass()) {
      Serial.print(F("# mass_g="));
      Serial.print(load.readMassGrams(), 2);
      Serial.println(F("  (UNCALIBRATED until set_scale with known mass)"));
    } else {
      Serial.println(F("# mass INVALID"));
    }
  } else if (cmd == "set_scale") {
    float f = arg.toFloat();
    if (f == 0.0f) {
      Serial.println(F("# usage: set_scale <nonzero factor>"));
    } else {
      load.setScaleFactor(f);
      Serial.print(F("# scale set to "));
      Serial.println(f, 4);
    }
  } else if (cmd == "adc") {
    int ch = ADS1115_TEST_CHANNEL;
    if (arg.length() > 0) ch = arg.toInt();
    if (ch < 0 || ch > 3) {
      Serial.println(F("# usage: adc [0..3]"));
    } else if (!adc.isHealthy()) {
      Serial.println(F("# ads1115 NOT PRESENT"));
    } else {
      int16_t r = adc.readRaw((uint8_t)ch);
      float v = adc.readVoltage((uint8_t)ch);
      Serial.print(F("# adc_ch="));
      Serial.print(ch);
      Serial.print(F(" raw="));
      Serial.print(r);
      Serial.print(F(" volts="));
      Serial.println(v, 4);
    }
  } else if (cmd == "adc_stats") {
    // syntax: adc_stats [ch] [n]
    int ch = ADS1115_TEST_CHANNEL;
    int n = ADS1115_AVG_SAMPLES;
    if (arg.length() > 0) {
      // naive split on space
      int sp2 = arg.indexOf(' ');
      if (sp2 < 0) {
        ch = arg.toInt();
      } else {
        ch = arg.substring(0, sp2).toInt();
        n = arg.substring(sp2 + 1).toInt();
      }
    }
    if (ch < 0 || ch > 3 || n <= 0 || n > 64) {
      Serial.println(F("# usage: adc_stats [0..3] [1..64]"));
    } else if (!adc.isHealthy()) {
      Serial.println(F("# ads1115 NOT PRESENT"));
    } else {
      AdcStats s = adc.readStats((uint8_t)ch, (uint8_t)n);
      Serial.print(F("# adc_stats ch="));
      Serial.print(ch);
      Serial.print(F(" n="));
      Serial.print(s.n);
      Serial.print(F(" mean="));
      Serial.print(s.mean);
      Serial.print(F(" min="));
      Serial.print(s.minv);
      Serial.print(F(" max="));
      Serial.print(s.maxv);
      Serial.print(F(" std="));
      Serial.print(s.stddev, 2);
      Serial.print(F(" volts_mean="));
      Serial.println(s.volts_mean, 4);
    }
  } else if (cmd == "sig") {
    if (sig == nullptr) {
      Serial.println(F("# no signal generator in this build"));
    } else if (arg.length() == 0) {
      Serial.print(F("# sig freq_hz="));
      Serial.print(sig->getFrequency(), 1);
      Serial.print(sig->isEnabled() ? F(" ON") : F(" OFF"));
      Serial.print(F(" src=ESP32-timer square 0..3.3V on GPIO"));
      Serial.println(SIGGEN_PIN);
    } else if (arg == "on") {
      sig->enable();
      Serial.println(F("# sig enabled"));
    } else if (arg == "off") {
      sig->disable();
      Serial.println(F("# sig disabled (pin parked at 0V)"));
    } else if (arg == "blink") {
      // Debug: slow 6x toggle so a multimeter can find the real pin.
      Serial.println(F("# blink: 6x 0.5s on GPIO26 — meter it now"));
      sig->disable();
      pinMode(SIGGEN_PIN, OUTPUT);
      for (int i = 0; i < 6; i++) {
        digitalWrite(SIGGEN_PIN, HIGH);
        delay(500);
        digitalWrite(SIGGEN_PIN, LOW);
        delay(500);
      }
      sig->enable();
      Serial.println(F("# blink done, sig re-enabled"));
    } else {
      float f = arg.toFloat();
      if (sig->setFrequency(f)) {
        Serial.print(F("# sig freq set to "));
        Serial.print(f, 1);
        Serial.println(F(" Hz"));
      } else {
        Serial.println(F("# usage: sig [5..1000 | on | off]"));
      }
    }
  } else if (cmd == "predict") {
    if (!adc.isHealthy()) {
      Serial.println(F("# ads1115 NOT PRESENT"));
    } else {
      AdcStats s = adc.readStats(ADS1115_TEST_CHANNEL, 48);
      float pct = lookupAddedEthanol(s.maxv);
      Serial.print(F("# predict max="));
      Serial.print(s.maxv);
      Serial.print(F(" added_ethanol_pct="));
      if (!isnan(pct)) {
        Serial.print(pct, 1);
        Serial.print(F(" (4PT table, base=market petrol)"));
      } else {
        Serial.print(F("OUT-OF-RANGE"));
      }
      Serial.println();
    }
  } else {
    Serial.println(F("# unknown command; type 'help'"));
  }
}
