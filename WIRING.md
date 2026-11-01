# EthanolBlend — Complete Wiring / Netlist (demo build)

Board: generic **ESP32 DOIT DevKit V1** (30-pin, ESP32-D0WD-V3), powered from laptop USB.
All logic is **3.3 V**. No 5 V logic anywhere in this build.

Scope of this document: exactly what is physically wired on the working demo rig
(branch `feat/esp32-excitation`). **HX711 and the load cell are NOT part of this build
and are excluded.** ICL8038 and LM358 are not installed either.

---

## 1. Modules used

| # | Module | Interface | Notes |
|---|--------|-----------|-------|
| 1 | ESP32 DOIT DevKit V1 | USB 5 V | also the excitation signal source |
| 2 | 0.96" OLED, SSD1306, 128x64 | I2C @ 0x3C | shares bus with ADS1115 |
| 3 | ADS1115 16-bit ADC breakout | I2C @ 0x48 | VDD = 3.3 V, gain ±4.096 V, 860 SPS |
| 4 | DS18B20 temperature sensor | 1-Wire | 12-bit, needs 4.7 kΩ pull-up |
| 5 | 10 kΩ resistor (¼ W) | — | upper leg of the electrode divider |
| 6 | Electrode cell (2 bare metal leads) | analog | lower leg of the divider |
| 7 | Tactile push-button | GPIO | optional, internal pull-up |

Not connected in this build: HX711, load cell, ICL8038 module, LM358, buck-boost module.

---

## 2. Power nets

Single common ground. The ESP32 is the only power source.

| From | To | Note |
|------|----|------|
| Laptop USB | ESP32 micro-USB | 5 V in, board regulator makes 3.3 V |
| ESP32 `3V3` | OLED `VCC` | |
| ESP32 `3V3` | ADS1115 `VDD` | must be 3.3 V, not 5 V |
| ESP32 `3V3` | DS18B20 `VDD` | 3-wire (non-parasitic) mode |
| ESP32 `3V3` | one end of 4.7 kΩ pull-up | other end goes to DS18B20 `DQ` |
| ESP32 `GND` | OLED `GND` | |
| ESP32 `GND` | ADS1115 `GND` | also the ADC's single-ended reference |
| ESP32 `GND` | DS18B20 `GND` | |
| ESP32 `GND` | button leg 2 | button is active-low |
| ESP32 `GND` | electrode cell, lead 2 | bottom of the divider |

---

## 3. I2C bus (shared, 400 kHz)

| Signal | ESP32 pin | OLED | ADS1115 |
|--------|-----------|------|---------|
| SDA | GPIO21 | SDA | SDA |
| SCL | GPIO22 | SCL | SCL |

Address select: **ADS1115 `ADDR` → GND** (gives 0x48). OLED is fixed 0x3C.
OLED `RESET` is left unconnected (module has no reset pin; firmware uses -1).

---

## 4. DS18B20 (1-Wire)

| DS18B20 pin | Connects to |
|-------------|-------------|
| VDD (pin 3) | ESP32 3V3 |
| GND (pin 1) | ESP32 GND |
| DQ (pin 2) | ESP32 **GPIO4** |
| — | **4.7 kΩ from DQ to 3V3** (required, bus will not enumerate without it) |

---

## 5. Excitation + electrode cell (the measurement front end)

ESP32 hardware timer generates a **20 Hz, 50 % duty, 0–3.3 V square wave** on **GPIO26**.
The cell is the *lower* leg of a resistive divider; the 10 kΩ is the *upper* leg.
The ADC taps the divider midpoint.

```
GPIO26 ──[ 10 kΩ ]──┬──● NODE N ──●── ADS1115 A0
                    │
              electrode 1
                    │
                 ( liquid )
                    │
              electrode 2
                    │
                   GND
```

| From | To |
|------|----|
| ESP32 **GPIO26** (signal out) | 10 kΩ, end A |
| 10 kΩ, end B | **node N** |
| node N | **ADS1115 `A0`** |
| node N | electrode 1 |
| electrode 2 | ESP32 GND (= ADS1115 GND) |
| ADS1115 `A1`, `A2`, `A3` | not connected |

Consequences of this topology (sanity checks for the schematic):

- Node voltage `V_N = 3.3 V × R_cell / (10 kΩ + R_cell)`.
- Dry / open cell → `V_N` → 3.3 V (~26400 ADC counts). Wet, conductive cell → `V_N` drops.
- Maximum current into the liquid is `3.3 V / 10 kΩ = 330 µA`, so the cell can never be
  driven hard and the ADC input can never exceed the 3.3 V rail.
- Nothing on A0 is ever outside 0…3.3 V by construction, so no clamping is fitted.

Lead convention (keep it fixed between readings for repeatability, carried over from the
earlier differential measurement): **WHITE lead = node N side (driven/sense),
GREEN lead = GND side.**

---

## 6. Measure button (optional)

| Button | Connects to |
|--------|-------------|
| leg 1 | ESP32 **GPIO13** |
| leg 2 | ESP32 GND |

Internal pull-up is enabled in firmware (`INPUT_PULLUP`); **no external resistor**.
Active-low, fires on release after ≥50 ms.

---

## 7. Firmware pin map (authoritative — `src/config.h`)

| GPIO | Function | Direction |
|------|----------|-----------|
| 21 | I2C SDA (OLED + ADS1115) | bidirectional |
| 22 | I2C SCL (OLED + ADS1115) | output |
| 4  | DS18B20 DQ (1-Wire) | bidirectional |
| 26 | Excitation square wave out | output |
| 13 | Measure button | input, pull-up |
| —  | 16, 17 | **unused** (were HX711 DOUT/SCK — removed) |

I2C clock 400 kHz. ADS1115 gain `GAIN_ONE` (±4.096 V, 125 µV/LSB), data rate 860 SPS.

---

## 8. Signal chain summary

```
ESP32 timer ── 20 Hz square ──> 10 kΩ ──> node N ──> ADS1115 A0 ──I2C──> ESP32
                                             │
                                        electrode cell
                                             │
                                            GND

ESP32 ──> OLED (bucket label + gauge + % + response + temperature)
ESP32 ──> USB serial @ 115200 (JSON lines + `predict` / `model`)
DS18B20 ──1-Wire──> ESP32 (temperature, logged and displayed)
```

Measurement: 48-sample burst at 860 SPS (~0.5 s) every 5 s; the **maximum** sample is the
lookup key, the mean is the reported response voltage.

---

## 9. Things a schematic must NOT show

- No HX711, no load cell, no strain-gauge bridge.
- No ICL8038, no LM358, no op-amp stage, no bipolar supply, no 12 V rail.
- No 5 V connection to the ADS1115 or its inputs.
- No connection to the ESP32 `EN` pin (reset line, on-board pull-up only).
- No external pull-ups on SDA/SCL beyond what the breakout boards already carry.
