# OPENCODE HANDOFF — ETHANOLBLEND

You are taking over development of an embedded hardware prototype.

Read this document completely before modifying code.

Also read:

    PROJECT_CONTEXT.md

PROJECT NAME:

    EthanolBlend

PROJECT PURPOSE:

    Build a low-cost embedded prototype capable of estimating ethanol
    concentration in controlled petrol–ethanol reference blends.

DEADLINE:

    2026-10-10

DEVELOPMENT WINDOW:

    Approximately 3–4 serious development days remain.

DEVELOPMENT STYLE:

    Hardware-first.
    Incremental.
    Test-driven where possible.
    Do not invent hardware.
    Do not over-engineer.

---

# 1. MOST IMPORTANT INSTRUCTION

DO NOT ASSUME HARDWARE THAT HAS NOT BEEN CONFIRMED.

The project originally planned to use:

- AD9833
- MCP3208

Neither is currently confirmed as physically available.

Potential alternatives include:

- XR2206
- ICL8038
- ADS1115
- MCP3008
- ADS1015
- ESP32 ADC

The exact parts will be supplied by the user as they become available.

The firmware architecture MUST therefore isolate hardware-specific implementations.

---

# 2. CURRENT HARDWARE

Known:

- ESP32
- HX711
- 1 kg load cell
- DS18B20 waterproof probe
- 0.96" 128×64 I2C OLED
- LM358 module or IC
- breadboard
- jumper wires
- resistors
- capacitors

Electrodes:

- currently likely bare component/wire leads
- dedicated stainless electrodes were not found

Not currently available:

- petrol
- ethanol
- beakers / proper sample containers
- confirmed dedicated ADC
- confirmed dedicated waveform generator

Therefore:

DO NOT attempt fuel calibration yet.

---

# 3. IMMEDIATE GOAL

Get a complete embedded firmware skeleton running on the ESP32.

The first target is:

    ESP32
      |
      +---- OLED
      |
      +---- DS18B20
      |
      +---- HX711 + load cell
      |
      +---- Serial protocol

Do NOT implement the electrical measurement subsystem until the actual ADC/signal-generation hardware is confirmed.

---

# 4. DEVELOPMENT ENVIRONMENT

Use:

- VS Code
- PlatformIO
- ESP32 Arduino framework unless there is a compelling reason otherwise
- C++
- Git

Do not migrate to ESP-IDF unless explicitly requested.

Keep dependencies minimal.

---

# 5. FIRST TASK

Inspect the repository.

If no project exists, create:

    firmware/

with PlatformIO configuration.

Use an appropriate ESP32 development board configuration.

If the exact ESP32 board model is unknown, use a generic ESP32 Arduino-compatible target and make board selection easy to change.

---

# 6. INITIAL PROJECT STRUCTURE

Create:

    firmware/
    ├── platformio.ini
    └── src/
        ├── main.cpp
        ├── config.h
        ├── sensors/
        │   ├── load_cell.h
        │   ├── load_cell.cpp
        │   ├── temperature.h
        │   └── temperature.cpp
        ├── display/
        │   ├── display.h
        │   └── display.cpp
        └── communication/
            ├── serial_protocol.h
            └── serial_protocol.cpp

Do not create 50 files for trivial functionality.

---

# 7. CONFIG.H

Create one central configuration file.

It should contain:

- GPIO pins
- I2C configuration
- HX711 pins
- DS18B20 pin
- OLED address
- sampling intervals
- serial baud rate
- feature flags

Example conceptual configuration:

    OLED_SDA_PIN
    OLED_SCL_PIN

    HX711_DOUT_PIN
    HX711_SCK_PIN

    DS18B20_PIN

Do not scatter pin numbers across source files.

If the user has not supplied exact GPIO wiring, use sensible defaults but make them clearly configurable and report the assumptions.

---

# 8. OLED DRIVER

Implement:

    Display::begin()
    Display::showBoot()
    Display::showSensorStatus(...)
    Display::showMeasurement(...)
    Display::showError(...)

The display layer should not contain sensor logic.

Initial boot screen:

    ETHANOLBLEND
    SYSTEM BOOT

Then:

    TEMP: --.- C
    MASS: --.-- g
    STATUS: OK

The UI should remain simple.

---

# 9. DS18B20 DRIVER

Implement:

    begin()
    readTemperature()
    isHealthy()

Return:

- temperature °C
- validity

Handle invalid readings.

Do not silently turn invalid sensor data into zero.

---

# 10. HX711 DRIVER

Implement:

    begin()
    tare()
    readRaw()
    readMassGrams()
    isStable()
    isHealthy()

The calibration factor MUST be configurable.

Do not hard-code a fake calibration factor and pretend it is accurate.

The firmware should support a calibration procedure.

Example serial commands:

    tare
    hx711_raw
    weight
    set_scale <value>

The exact command interface may be improved if necessary.

---

# 11. SENSOR STATE

Maintain a central measurement structure.

Conceptually:

    struct Measurement {
        uint32_t timestamp_ms;

        float temperature_c;
        bool temperature_valid;

        float mass_g;
        bool mass_valid;

        float volume_ml;
        float density_g_ml;

        float excitation_frequency_hz;
        float excitation_amplitude;

        float electrical_response;
        bool electrical_valid;
    };

Do not calculate density unless volume is known.

Do not invent volume.

---

# 12. SERIAL OUTPUT

Provide human-readable debug output.

Also provide machine-readable measurement output.

Preferred:

JSON Lines.

Example:

    {"type":"measurement","temperature_c":26.3,"mass_g":72.2}

Do not mix arbitrary debug text into machine-readable output mode.

Provide a debug flag.

---

# 13. MAIN LOOP

The main loop should be non-blocking where practical.

Avoid:

    delay(5000)

for ordinary sensor handling.

Use millis()-based scheduling.

Suggested tasks:

- sensor update
- display update
- serial processing
- measurement collection

---

# 14. ELECTRICAL MEASUREMENT ARCHITECTURE

DO NOT IMPLEMENT UNTIL HARDWARE IS CONFIRMED.

Potential architecture:

    Signal generator
         |
         v
    current limiting /
    attenuation
         |
         v
    electrode cell
         |
         v
    LM358 conditioning
         |
         v
    ADC
         |
         v
    ESP32

The actual circuit depends on:

- signal generator
- ADC
- LM358 configuration
- electrode geometry

---

# 15. ADC ABSTRACTION

Create:

    adc.h
    adc.cpp

with a hardware-neutral API.

Conceptually:

    begin()
    readChannel(channel)
    readVoltage(channel)
    isHealthy()

Potential implementations:

    ESP32 ADC
    MCP3208
    MCP3008
    ADS1115
    ADS1015

Only compile the implementation corresponding to the selected hardware.

The rest of the firmware must not care.

---

# 16. SIGNAL GENERATOR ABSTRACTION

Similarly, create:

    signal_generator.h
    signal_generator.cpp

Conceptual API:

    begin()
    setFrequency(float hz)
    setAmplitude(float value)
    enable()
    disable()
    getFrequency()
    getAmplitude()

Possible implementation:

    external_manual_generator

For XR2206/ICL8038 modules where frequency/amplitude is manually controlled, the firmware should treat the generator as externally configured.

Do NOT pretend the ESP32 can control the physical potentiometers unless actual digital control hardware exists.

---

# 17. VERY IMPORTANT: MANUAL GENERATOR MODE

If the user obtains an XR2206 or ICL8038 module without digital control:

The ESP32 cannot magically set its frequency.

The correct workflow is:

1. User manually adjusts generator.
2. User measures/sets the desired frequency.
3. Firmware stores the expected frequency as metadata.
4. Measurements are recorded at that fixed frequency.

Example:

    excitation_frequency_hz = 1000

This value should only be treated as accurate if the user actually verified it.

---

# 18. ESP32 ADC FALLBACK

If no external ADC is obtained:

Use ESP32 ADC.

The code should support:

    ADC_TYPE = ESP32

Do not claim 12-bit laboratory-grade performance.

The firmware should support averaging.

Potential:

    N = 32 or 64 samples

Then calculate:

- mean
- minimum
- maximum
- standard deviation

This provides basic measurement stability information.

---

# 19. ELECTRICAL FEATURES

The system may eventually extract:

- raw ADC mean
- raw ADC RMS
- peak-to-peak
- response amplitude
- excitation/response ratio
- apparent resistance/impedance proxy
- frequency

Only implement mathematically valid features based on actual hardware.

Do NOT call a raw ADC amplitude "impedance" unless the circuit actually permits that calculation.

Use precise terminology such as:

    electrical response
    amplitude ratio
    apparent impedance proxy

when appropriate.

---

# 20. TESTING WITHOUT FUEL

The user currently has no petrol or ethanol.

Therefore, test the analog chain using known resistors.

Potential test resistors:

    1 kΩ
    10 kΩ
    47 kΩ
    100 kΩ

Goal:

Change the test resistance and verify that the acquisition system changes predictably.

This tests:

- signal generation
- analog front end
- ADC
- filtering
- serial output

It does NOT validate fuel analysis.

---

# 21. TESTING WITH WATER

If distilled water becomes available, it may be used as a basic sample-cell test.

Purpose:

- electrode continuity
- response stability
- noise
- repeatability

Do not claim water response represents petrol or ethanol response.

---

# 22. CALIBRATION SOFTWARE

The PC-side software should be implemented before the fuel arrives.

It should support synthetic/dummy datasets.

Example dummy dataset:

    E0
    E10
    E20
    E30
    E50

with multiple readings each.

The dummy data must be explicitly marked:

    synthetic = true

Never present synthetic data as experimental data.

---

# 23. PYTHON ANALYZER

Create:

    analyzer/

with:

    serial_reader.py
    dataset.py
    features.py
    calibration.py
    model.py
    predict.py
    cli.py

The analyzer should eventually support:

    record
    calibrate
    evaluate
    predict

Example:

    python cli.py record

    python cli.py calibrate data/calibration.csv

    python cli.py evaluate data/calibration.csv

    python cli.py predict sample.json

---

# 24. MODELING

Start simple.

Implement:

1. baseline mean/interpolation
2. linear regression
3. polynomial regression
4. optional Random Forest
5. optional Gradient Boosting

Evaluate using:

- MAE
- RMSE
- R² where appropriate

Use train/test separation.

With small datasets, use cross-validation rather than pretending a tiny dataset gives strong generalization.

---

# 25. DATA VALIDATION

Reject:

- impossible temperatures
- missing mass
- negative mass
- invalid ADC readings
- impossible volumes
- invalid density
- missing calibration labels

Do not silently drop data without reporting it.

---

# 26. CONFIDENCE

Never fabricate confidence.

If the model validation gives:

    MAE = 1.7 percentage points

the UI may display:

    Expected error ≈ ±1.7 percentage points

subject to the actual evaluation methodology.

If insufficient data exists:

    Confidence: LOW

---

# 27. CALIBRATION PROCEDURE

Eventually:

For each reference blend:

    1. Prepare sample.
    2. Measure volume.
    3. Place sample container on load cell.
    4. Record mass.
    5. Record temperature.
    6. Place electrodes.
    7. Record electrical response.
    8. Repeat multiple times.
    9. Save all measurements.
    10. Move to next concentration.

Avoid changing:

- electrode spacing
- electrode depth
- sample volume
- measurement timing
- excitation frequency

between calibration samples unless explicitly intended.

---

# 28. ELECTRODE GEOMETRY

Because dedicated electrodes were unavailable:

Current fallback:

    bare conductive wire/component leads

Requirements:

- same material
- same exposed length
- same spacing
- same immersion depth
- rigid mounting

The geometry is part of the sensor.

Therefore it must be reproducible.

If the user changes the geometry, recalibration is required.

---

# 29. SAMPLE VOLUME

Use a fixed sample volume for initial calibration.

The exact volume depends on available containers and measuring equipment.

Make it configurable.

Do not assume 100 mL.

---

# 30. TEMPERATURE COMPENSATION

Temperature must be recorded.

Initial model:

    ethanol_percent =
        f(electrical_response,
          density,
          temperature)

Do not attempt sophisticated physical temperature compensation before sufficient data exists.

Let the calibration model learn temperature dependence if the dataset supports it.

---

# 31. SOFTWARE QUALITY

Every meaningful change must:

1. Compile.
2. Run static checks if available.
3. Avoid introducing unrelated changes.
4. Be committed logically.

For Python:

- use a virtual environment
- use type hints where reasonable
- use pytest for core calculations

For firmware:

- build with PlatformIO

---

# 32. ACCEPTANCE CRITERIA — PHASE 1

Phase 1 passes when:

- PlatformIO project builds
- ESP32 boots
- serial output works
- OLED initializes
- DS18B20 returns temperature
- HX711 returns load-cell readings
- load cell can be tared
- measurement structure exists
- code is modular
- no unavailable hardware is assumed

---

# 33. ACCEPTANCE CRITERIA — PHASE 2

Phase 2 passes when:

- actual ADC is integrated
- signal source is integrated
- analog measurement chain works
- known resistors produce distinguishable readings
- ADC noise is characterized
- readings are repeatable

---

# 34. ACCEPTANCE CRITERIA — PHASE 3

Phase 3 passes when:

- electrode cell is assembled
- sample measurement is repeatable
- temperature is recorded
- mass/density is recorded
- electrical response is recorded
- data is saved correctly

---

# 35. ACCEPTANCE CRITERIA — PHASE 4

Phase 4 passes when:

- calibration samples exist
- repeated measurements exist
- model is trained
- validation is performed
- ethanol estimate is displayed
- uncertainty/error metric is displayed

---

# 36. FINAL DEMO

The final device should have a simple workflow:

    POWER ON
       ↓
    SYSTEM CHECK
       ↓
    INSERT SAMPLE
       ↓
    MEASURE
       ↓
    STABILIZE
       ↓
    ANALYZE
       ↓
    RESULT

OLED example:

    ETHANOLBLEND

    Measuring...

    Temp: 26.4 C
    Mass: 72.1 g

Then:

    ETHANOL

    20.8 %

    Error: ±2.0 %

The actual error must come from validation.

---

# 37. DON'T OVERENGINEER

This project has a hard deadline.

Do NOT build:

- mobile app
- cloud backend
- authentication
- web dashboard
- database server
- complex enclosure
- OTA updates
- BLE unless genuinely useful
- unnecessary AI agents

The core instrument is the priority.

---

# 38. HOW TO WORK WITH THE USER

The user is physically building the hardware while OpenCode develops software.

Therefore, when hardware is required:

Explain:

    WHAT TO CONNECT
    WHERE TO CONNECT IT
    WHAT TO MEASURE
    WHAT RESULT SHOULD APPEAR
    WHAT TO SEND BACK IF IT FAILS

Example:

    Connect HX711:
        VCC → 3.3V
        GND → GND
        DT  → GPIO XX
        SCK → GPIO XX

Then provide:

    Expected serial:
    HX711 OK
    raw=...
    mass=...

Do not assume the user has already wired something.

---

# 39. WHEN HARDWARE IS UNKNOWN

Ask the user for:

- exact part name
- photo
- datasheet/link
- pin labels

before writing hardware-specific code.

Never invent a pinout from memory if the module variant is ambiguous.

---

# 40. CURRENT PRIORITY ORDER

Work in exactly this order unless a hardware issue requires deviation:

    1. PlatformIO project
    2. ESP32 boot
    3. Serial logging
    4. OLED
    5. DS18B20
    6. HX711
    7. Load-cell calibration
    8. Serial measurement protocol
    9. ADC abstraction
    10. Identify actual ADC
    11. Identify actual signal generator
    12. Analog acquisition
    13. Electrical test using resistors
    14. Electrode cell
    15. Water/basic repeatability test
    16. Fuel calibration
    17. Regression
    18. Final OLED UI
    19. Final demonstration

---

# 41. FIRST COMMAND TO EXECUTE

Before implementing anything:

1. Inspect repository.
2. Read PROJECT_CONTEXT.md.
3. Determine whether a PlatformIO project already exists.
4. Determine actual connected/available hardware.
5. Do not modify unrelated files.
6. Report the current repository state.
7. Propose Phase 1 implementation.
8. Implement Phase 1.
9. Run PlatformIO build.
10. Fix errors.
11. Report exactly what was implemented and what remains.

Do not skip the repository inspection step.

---

# 42. FINAL RULE

The project should always prefer:

    REAL HARDWARE
        +
    REAL MEASUREMENTS
        +
    REAL CALIBRATION

over:

    simulated results
    invented accuracy
    unnecessary complexity
    impressive-looking but unverified features

The objective is a working physical prototype, not merely a software demo.