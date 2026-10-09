// serial_protocol.h — human debug + machine JSON Lines (Phase 1)

#pragma once

#include <Arduino.h>

struct Measurement;
class TemperatureSensor;
class LoadCell;
class Ads1115Reader;
class SignalGenerator;

class SerialProtocol {
 public:
  void begin(unsigned long baud);
  // Call every loop with live driver refs; handles incoming commands.
  void handleCommands(TemperatureSensor& temp, LoadCell& load, Ads1115Reader& adc,
                      SignalGenerator* sig = nullptr);
  // Machine-readable JSON Lines measurement (one line per call).
  void publishMeasurement(const Measurement& m);
  void printBootBanner();
  void printStatus(bool temp_ok, bool hx_ok, bool ads_ok, bool oled_ok);

 private:
  void printHelp();
  String readLine();
};
