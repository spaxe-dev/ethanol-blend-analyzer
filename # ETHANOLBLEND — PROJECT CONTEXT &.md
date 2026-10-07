# ETHANOLBLEND — PROJECT CONTEXT & TECHNICAL SOURCE OF TRUTH

Version: 0.1
Project status: Pre-calibration / electronics development
Target demonstration: 2026-10-10
Development environment: Windows + VS Code + PlatformIO + OpenCode
Primary MCU: ESP32
Project type: Embedded hardware + signal measurement + calibration software

---

# 1. PROJECT OVERVIEW

EthanolBlend is a low-cost embedded prototype intended to estimate the ethanol percentage of petrol–ethanol fuel blends.

The project combines:

1. Electrical response measurement
2. Density measurement
3. Temperature measurement
4. Embedded data acquisition
5. Calibration against known reference mixtures
6. A software calibration/regression layer
7. OLED/local embedded display
8. Optional laptop-side analysis and visualization

The prototype is NOT intended to be a laboratory-grade fuel analyzer.

The correct claim is:

> A low-cost calibrated prototype for estimating ethanol concentration in controlled petrol–ethanol reference blends.

The system must NOT claim:

- certified fuel testing
- laboratory-grade accuracy
- universal compatibility with arbitrary commercial fuels
- guaranteed ethanol percentage in unknown fuel without calibration
- legal/metrology certification
- chemical purity analysis

The project is primarily an engineering prototype demonstrating that multiple measurable physical properties can be combined into an embedded estimation system.

---

# 2. CORE IDEA

The physical idea is to measure properties of a fuel sample that vary with composition.

The main intended features are:

- Electrical response between electrodes
- Sample density
- Temperature

The software then maps these measurements to ethanol concentration.

Conceptually:

    Sample
       |
       +----------------------+
       |                      |
       v                      v
 Electrical response       Density
       |                      |
       +----------+-----------+
                  |
              Temperature
                  |
                  v
          Feature extraction
                  |
                  v
        Calibration / regression
                  |
                  v
       Estimated ethanol %
                  |
                  v
             OLED / PC

---

# 3. IMPORTANT PROJECT PRINCIPLES

## 3.1 Calibration is central

The analyzer is not expected to derive ethanol concentration from a single universal equation.

Instead, it should be calibrated experimentally.

Potential reference samples:

- E0
- E10
- E20
- E30
- E50

where E20 means approximately 20% ethanol by volume and the remainder is petrol.

The actual available reference concentrations will depend on what materials can be obtained.

The firmware and software must therefore support arbitrary calibration labels rather than hard-code only E10/E20/etc.

---

# 4. CURRENT HARDWARE STATUS

## Already owned

- ESP32 development board
- Laptop
- USB cable
- Soldering iron
- Solder
- Multimeter
- Basic hand tools

## Purchased / intended

- 1 kg load cell
- 7Semi HX711 module
- DS18B20 waterproof temperature probe
- 0.96" 128×64 I2C OLED
- LM358 module or LM358 IC
- Breadboard
- Jumper wires
- Resistors
- Capacitors
- Miscellaneous mechanical materials

## Electrode situation

Dedicated laboratory electrodes could not be found.

The current fallback is to use bare conductive wire/component leads as temporary electrodes.

This is acceptable for the crude prototype provided:

- both electrodes use the same material
- both have the same exposed length
- electrode spacing is fixed
- immersion depth is fixed
- geometry is mechanically constrained
- electrodes are cleaned consistently between samples

Preferred future material:

- stainless steel wire
- stainless steel rod
- SS304 or SS316 if obtainable

Do not make expensive laboratory electrodes a dependency.

---

# 5. SIGNAL GENERATION STATUS

The original preferred excitation source was:

AD9833 DDS module.

The AD9833 module could not be sourced locally.

Alternative candidates:

- XR2206 module
- ICL8038 module
- another suitable low-cost function generator
- ESP32-generated waveform as fallback

IMPORTANT:

The project must NOT assume that AD9833 exists.

The actual signal-generator hardware must be determined from what is physically available.

If a ready-made XR2206 or ICL8038 module is obtained, the firmware/measurement design should accommodate it.

If neither is available, investigate ESP32 waveform generation.

Do not waste project time waiting for AD9833.

---

# 6. ADC STATUS

The original preferred external ADC was:

MCP3208.

MCP3208 has not been obtained.

Potential alternatives:

- MCP3208
- ADS1115
- MCP3008
- ADS1015
- ESP32 internal ADC as final fallback

Priority:

1. MCP3208
2. ADS1115
3. MCP3008
4. ADS1015
5. ESP32 ADC

The firmware must NOT hard-code MCP3208 assumptions until the physical ADC is confirmed.

ADC abstraction is mandatory.

---

# 7. HARDWARE ARCHITECTURE

The intended architecture is:

SIGNAL GENERATOR
        |
        v
AMPLITUDE / ATTENUATION
        |
        v
ELECTRODE CELL
        |
        v
ANALOG SIGNAL CONDITIONING
        |
        v
ADC
        |
        v
ESP32
        |
        +-------- HX711 / Load Cell
        |
        +-------- DS18B20
        |
        +-------- OLED
        |
        +-------- USB serial / PC

The exact circuit is intentionally not frozen until the actual signal generator and ADC are known.

---

# 8. ELECTRICAL MEASUREMENT PRINCIPLE

The system needs a controlled AC excitation through the sample.

Two electrodes are placed in the sample.

The electrical response of the sample is measured.

At minimum, the system can characterize an amplitude-related response.

Potential features:

- excitation frequency
- excitation amplitude
- measured response amplitude
- ratio between excitation and response
- apparent impedance-related feature
- phase-related feature if the hardware permits it

The system does NOT need to perform full laboratory impedance spectroscopy.

A single fixed frequency can be sufficient for the initial prototype if calibration data demonstrates useful separation.

If multiple frequencies are practical, the architecture should permit frequency sweeps later.

---

# 9. IMPORTANT ELECTRICAL SAFETY / DESIGN RULES

The electrode cell must use low-voltage electrical excitation.

Do not expose fuel to:

- mains voltage
- high voltage
- sparks
- heating elements
- exposed hot components
- open flames

The prototype should use very small sample quantities.

The excitation must be current-limited.

The electrode cell should be designed so the electrical path cannot accidentally become a short circuit that damages the generator or ADC.

Do not place the signal generator output directly into an unknown sample without appropriate current limiting.

---

# 10. DENSITY MEASUREMENT

Density is calculated from:

    density = mass / volume

The load cell measures mass.

A known sample volume is used.

For example:

    mass = 72.5 g
    volume = 100 mL

    density = 0.725 g/mL

The prototype may use a fixed sample volume to simplify operation.

The exact sample volume must be configurable.

Do not hard-code 100 mL into every module.

---

# 11. LOAD CELL SYSTEM

Expected hardware:

- 1 kg load cell
- HX711 amplifier

The ESP32 communicates with HX711 using digital GPIO.

Firmware should provide:

- tare
- raw reading
- calibrated grams
- stability detection
- zero detection
- error/disconnection detection if possible

A calibration factor must be stored/configurable.

Do not assume the load cell calibration factor before performing actual calibration.

---

# 12. TEMPERATURE SYSTEM

Sensor:

DS18B20 waterproof temperature probe.

Temperature should be included because electrical properties can be temperature-dependent.

The firmware should expose:

- temperature in °C
- sensor status
- validity flag

Temperature must be recorded alongside every measurement.

---

# 13. OLED

Expected display:

0.96" 128×64 I2C OLED.

The OLED should eventually display:

Idle:
    ETHANOLBLEND
    Insert sample

During measurement:
    Measuring...
    Temp: XX.X C
    Mass: XX.X g

Final:
    Ethanol: XX %
    Confidence: XX %

Optional:
    Density
    Electrical response
    Status

The OLED should not become responsible for the core logic.

Use a display abstraction.

---

# 14. FIRMWARE ARCHITECTURE

Use PlatformIO.

Language:

C++.

Target:

ESP32.

Suggested architecture:

firmware/
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── config.h
│   │
│   ├── sensors/
│   │   ├── load_cell.cpp
│   │   ├── load_cell.h
│   │   ├── temperature.cpp
│   │   ├── temperature.h
│   │   ├── adc.cpp
│   │   ├── adc.h
│   │   ├── electrical.cpp
│   │   └── electrical.h
│   │
│   ├── display/
│   │   ├── display.cpp
│   │   └── display.h
│   │
│   ├── calibration/
│   │   ├── calibration.cpp
│   │   └── calibration.h
│   │
│   └── communication/
│       ├── serial_protocol.cpp
│       └── serial_protocol.h
│
└── include/

Do not create an unnecessarily huge architecture.

Modularity is required because the hardware is still changing.

---

# 15. HARDWARE ABSTRACTION

The code should not assume a specific ADC.

Use an interface conceptually similar to:

    class AnalogReader {
        public:
            virtual bool begin() = 0;
            virtual float readVoltage() = 0;
            virtual bool healthy() = 0;
    };

The actual implementation may be:

- MCP3208Reader
- ADS1115Reader
- MCP3008Reader
- ESP32AdcReader

Similarly, signal generation should be abstracted.

Potential implementations:

- XR2206ExternalGenerator
- ICL8038ExternalGenerator
- Esp32WaveformGenerator

The system should compile even when one implementation is disabled.

---

# 16. CONFIGURATION

All hardware-specific configuration should be centralized.

Example:

    // config.h

    #define OLED_ENABLED true
    #define HX711_ENABLED true
    #define DS18B20_ENABLED true

    #define ADC_TYPE ESP32_ADC

    #define SIGNAL_GENERATOR_TYPE EXTERNAL

GPIO pins should be defined in one location.

Do not scatter pin numbers across source files.

---

# 17. SERIAL PROTOCOL

The ESP32 should provide machine-readable serial output.

Recommended format:

JSON Lines.

Example:

    {
      "type": "measurement",
      "temperature_c": 26.4,
      "mass_g": 71.8,
      "adc_raw": 1832,
      "response": 0.421
    }

A simpler CSV mode may also be provided.

The PC-side software should not need to parse human-readable debug logs.

Separate:

- DEBUG
- DATA

messages.

---

# 18. PC-SIDE SOFTWARE

Python is recommended.

Structure:

analyzer/
├── requirements.txt
├── calibration.py
├── dataset.py
├── features.py
├── model.py
├── predict.py
├── serial_reader.py
└── cli.py

Responsibilities:

- receive serial measurements
- save CSV/JSONL
- clean data
- calculate density
- extract electrical features
- train calibration models
- evaluate models
- predict ethanol percentage
- calculate uncertainty/confidence
- export calibration data

---

# 19. DATA MODEL

Each measurement should contain at minimum:

sample_id
timestamp
temperature_c
mass_g
volume_ml
density_g_ml
excitation_frequency_hz
excitation_amplitude
electrical_response
known_ethanol_percent

The final field may be null for unknown samples.

Example:

    sample_id,E20_01
    temperature_c,26.2
    mass_g,72.1
    volume_ml,100
    density_g_ml,0.721
    excitation_frequency_hz,1000
    excitation_amplitude,0.5
    electrical_response,0.42
    known_ethanol_percent,20

---

# 20. CALIBRATION WORKFLOW

Once petrol and ethanol are available:

1. Prepare controlled reference blends.
2. Measure temperature.
3. Measure sample mass at known volume.
4. Measure electrical response.
5. Repeat measurements.
6. Record known ethanol percentage.
7. Build calibration dataset.
8. Train model.
9. Evaluate using held-out samples.
10. Only then use unknown samples.

Repeated measurements are important.

Do not train on one reading per blend.

Prefer several repeated measurements per concentration.

---

# 21. INITIAL MODEL

Do NOT start with complicated ML.

First implement:

- linear regression
- polynomial regression if justified
- nearest-neighbor / interpolation for discrete blend classes

Then evaluate:

- Random Forest
- Gradient Boosting

Only use a more complicated model if validation demonstrates improvement.

The goal is not to claim "AI".

The goal is a reliable calibrated mapping.

---

# 22. CONFIDENCE

Confidence should be derived from actual model uncertainty/error.

Do NOT output arbitrary confidence values.

Bad:

    Confidence = 97%

with no statistical basis.

Better:

    Estimated ethanol: 21.3%
    Validation MAE: 1.8 percentage points

or:

    Estimated ethanol: 21.3%
    Prediction interval: ±2.4%

If there is insufficient calibration data:

    Confidence: LOW

---

# 23. CURRENT DEVELOPMENT STATE

IMPORTANT:

At the beginning of coding, there may be NO petrol, ethanol, or beakers available.

This is intentional.

Development should proceed without them.

Current phase:

    Electronics + firmware bring-up

Not:

    Fuel calibration

---

# 24. DEVELOPMENT WITHOUT FUEL

The system can be developed using:

- known resistors
- water
- room-temperature tests
- dummy calibration data
- simulated sensor data

For electrical measurement development:

    signal source
       |
       v
    known resistor
       |
       v
    analog front end
       |
       v
    ADC

Test values such as:

- 1 kΩ
- 10 kΩ
- 47 kΩ
- 100 kΩ

can be used to verify acquisition and software.

Do not claim these are fuel measurements.

---

# 25. CURRENT FIRST MILESTONE

The first complete milestone should be:

    ESP32
      |
      +-- OLED works
      |
      +-- DS18B20 works
      |
      +-- HX711 works
      |
      +-- Load cell produces stable readings
      |
      +-- Serial protocol works
      |
      +-- Firmware builds reproducibly

Only after this should the electrical measurement subsystem be integrated.

---

# 26. GIT

Use Git from the beginning.

Suggested commits:

    chore: initialize platformio project
    feat: add oled driver
    feat: add ds18b20 driver
    feat: add hx711 driver
    feat: add serial measurement protocol
    feat: add adc abstraction
    feat: add electrical measurement
    feat: add calibration dataset pipeline
    feat: add regression model

Never make one enormous commit containing everything.

---

# 27. OPEN-CODE RULES

OpenCode must follow these rules:

1. Do not invent hardware.
2. Do not assume unavailable components.
3. Do not change hardware architecture without explaining why.
4. Do not fabricate sensor readings.
5. Do not claim hardware was tested unless the user actually tested it.
6. Do not claim calibration exists before calibration data exists.
7. Do not claim accuracy without validation.
8. Do not add unnecessary dependencies.
9. Do not rewrite working modules unnecessarily.
10. Compile after meaningful firmware changes.
11. Keep hardware configuration centralized.
12. Prefer simple code over clever code.
13. Never hide compilation errors.
14. Never silently substitute a different sensor.
15. Ask the user for the exact part number when hardware ambiguity affects implementation.

---

# 28. HARDWARE UNCERTAINTY RULE

The exact ADC and signal generator are currently uncertain.

Therefore:

DO NOT write code that assumes:

    AD9833
    MCP3208
    XR2206
    ICL8038

is physically connected.

First establish the actual hardware.

If the user says:

    "I bought ADS1115"

then implement ADS1115.

If the user says:

    "I bought MCP3008"

then implement MCP3008.

If the user says:

    "I have nothing"

then use ESP32 ADC fallback.

---

# 29. MEASUREMENT SAFETY

The prototype uses flammable fuel.

Rules:

- No flames.
- No smoking.
- No sparks.
- No heating.
- No mains electricity near sample.
- Use tiny samples.
- Work in a ventilated area.
- Keep fuel away from hot soldering equipment.
- Keep laptop/power supplies away from open fuel containers where practical.
- Never perform experiments with energized exposed conductors directly above an open fuel container.
- Disconnect power before changing electrode wiring.
- Keep sample containers closed when not measuring.

The electrical excitation must be low voltage and current limited.

---

# 30. SUCCESS CRITERIA

The project is successful if it can demonstrate:

1. Stable ESP32 firmware.
2. Working temperature measurement.
3. Working mass measurement.
4. Working OLED.
5. Working electrical response measurement.
6. Repeatable measurements.
7. Calibration dataset from known blends.
8. A measurable relationship between features and ethanol concentration.
9. Software prediction of ethanol percentage.
10. A clear physical demonstration.

The project is NOT required to achieve laboratory-grade accuracy.

---

# 31. DEMONSTRATION STORY

The final demonstration should communicate:

Problem:

Fuel ethanol content matters and composition can vary.

Approach:

Measure multiple physical signatures instead of relying on a single sensor.

System:

    Electrical response
    +
    Density
    +
    Temperature
    ↓
    Calibration model
    ↓
    Ethanol estimate

Output:

    ETHANOL
    20.8 %
    ±X.X %

The ± value must come from validation.

---

# 32. THINGS WE MUST NOT DO

Do not:

- use an expensive LCR meter as the final system
- require laboratory equipment
- depend on AD9833
- depend on MCP3208
- depend on specialized commercial electrodes
- add cameras
- add unnecessary AI
- add pumps
- add motors
- add valves
- build a complex enclosure before electronics work
- build a complicated mobile app before the sensor works
- claim legal accuracy
- fabricate calibration results

---

# 33. CURRENT PROJECT PHILOSOPHY

Build the simplest working instrument first.

Priority:

    Physical measurement
        >
    stable firmware
        >
    calibration
        >
    prediction
        >
    UI polish

A beautiful interface with unreliable measurements is a failed project.

A crude physical prototype with repeatable measurements is a successful engineering prototype.