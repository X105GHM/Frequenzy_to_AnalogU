# ATtiny85 Frequency-to-Voltage Converter

> **Disclaimer:** This README was written with the help of **ChatGPT 5.1**.

This project converts an **input frequency** into a **proportional analog voltage** using an **ATtiny85**.

The input signal frequency is measured via the **ADC**, then mapped to a **PWM output**.  
A simple **RC low-pass filter** converts the PWM signal into a smooth analog voltage.

This works with different waveform types, including:

- sine wave
- triangle wave
- sawtooth
- square wave

---

## Features

- **ATtiny85 (8 MHz)** based
- **No Arduino framework** (bare-metal AVR / `avr-libc`)
- Modern **C++17**
- Clean project structure with separate `.cpp` / `.hpp` files
- ADC-based frequency measurement with:
  - threshold detection
  - hysteresis
  - signal timeout detection
  - basic smoothing (IIR filter)
- PWM output on **OC0A**
- Analog output via **PWM + RC filter**

---

## How It Works

1. The ATtiny85 samples the input signal using the **ADC in free-running mode**.
2. The code detects **rising threshold crossings** (with hysteresis) to estimate the signal period.
3. The measured period is converted into a **frequency (Hz)**.
4. The frequency is mapped to a **PWM duty cycle (0–255)**.
5. A simple **RC filter** converts the PWM signal into a smooth analog voltage.

### Important note

This code does **not detect the waveform type** (sine, triangle, square, etc.).  
It only measures how often the signal crosses a threshold, so it works with many waveforms as long as:

- the signal is within the ADC range (**0 V to VCC**)
- the amplitude is large enough
- the signal crosses the threshold reliably

---

## Pin Mapping (ATtiny85 DIP-8)

### Input (frequency signal)

- **PB2 / ADC1** → **Pin 7**

### Output (PWM)

- **PB0 / OC0A** → **Pin 5**

### Power

- **VCC** → Pin 8
- **GND** → Pin 4

---

## Signal Input Requirements

The ADC can only measure voltages from **0 V to VCC**.

If your input is an AC signal (for example a sine wave centered around 0 V), you must:

- shift it to a **DC bias** (typically **VCC/2**)
- optionally use a coupling capacitor
- protect the ADC pin if the source can exceed the allowed voltage range

### Recommended for sine/triangle signals

Use a **bias network** so the waveform is centered around **VCC/2** (e.g. 2.5 V at 5 V supply).

---

## Analog Output (PWM + RC Filter)

The output is PWM and should be filtered to obtain a DC voltage.

### Example RC values

- **R = 10 kΩ**
- **C = 100 nF** (or **1 µF** for stronger smoothing)

The PWM runs at about **31.25 kHz** (Timer0, Fast PWM, prescaler = 1 at 8 MHz), which is well suited for RC filtering.

---

## Project Structure

```text
.
│   main.cpp                           # Main application loop
│
├───app
│       FrequencyToPwmMapper.hpp       # Frequency -> PWM mapping logic
│
├───config
│       config.hpp                     # Central configuration (ADC, thresholds, mapping, limits)
│
└───drivers
        AdcFrequencyMeter.cpp          # ADC setup, ISR, signal measurement, filtering
        AdcFrequencyMeter.hpp          # ADC frequency measurement interface
        PwmOutput.cpp                  # Timer0 PWM setup and duty control
        PwmOutput.hpp                  # PWM driver interface
```

---

## Code Overview

### `config/config.hpp`

Contains all user-adjustable parameters:

- ADC settings
- threshold and hysteresis values
- valid frequency range
- frequency-to-PWM mapping range
- timeout for signal loss

This makes it easy to tune the behavior without touching the driver code.

---

### `drivers/PwmOutput`

Responsible for configuring and controlling the PWM output:

- sets **PB0 (OC0A)** as output
- configures **Timer0** in Fast PWM mode
- updates duty cycle via `setDuty()`

---

### `drivers/AdcFrequencyMeter`

Responsible for measuring the frequency using the ADC:

- ADC runs in **free-running mode**
- interrupt (`ADC_vect`) reads samples continuously
- hysteresis-based threshold detection identifies rising edges
- measured period is converted to frequency
- basic IIR smoothing stabilizes the output

---

### `app/FrequencyToPwmMapper.hpp`

Maps frequency (Hz) to PWM (0–255) using a linear scale.

Example behavior:

- frequency below `kMapFreqMinHz` → PWM = 0
- frequency above `kMapFreqMaxHz` → PWM = 255
- values in between are linearly scaled

---

### `main.cpp`

The main loop is intentionally simple:

- read measured frequency
- map frequency to PWM
- update PWM output
- if no valid signal is detected → output 0 V (PWM duty = 0)

---

## Build Environment

This project uses **PlatformIO** with **bare-metal AVR** (no Arduino framework).

### `platformio.ini`

> `src_dir = .` is included because `main.cpp` is stored in the project root.

```ini
[env:attiny85]
platform = atmelavr
board = attiny85
framework =
src_dir = .
upload_protocol = avrispmkII

board_build.f_cpu = 8000000L

build_flags =
    -DF_CPU=8000000UL
    -std=gnu++17
    -Wall
    -Wextra
    -Wconversion
    -Werror=format
    -Os
    -ffunction-sections
    -fdata-sections
    -fno-exceptions
    -fno-rtti
    -Wl,--gc-sections

board_fuses.lfuse = 0xE2
board_fuses.hfuse = 0xDF
board_fuses.efuse = 0xFF
```

### Fuse note

The fuse values above configure the ATtiny85 for **8 MHz internal oscillator** (no clock divide by 8), which is required for correct timing and frequency measurement.

---

## Tuning and Calibration

You can adjust the behavior in `config/config.hpp`.

### Frequency mapping range

Defines which input frequency range corresponds to **0…100% PWM** (and therefore **0…VCC** after RC filtering).

```cpp
kMapFreqMinHz
kMapFreqMaxHz
```

### Threshold / Hysteresis

Useful if your signal amplitude or noise level changes.

```cpp
kThreshold
kHysteresis
```

### Valid frequency limits

Used to reject invalid or unstable measurements.

```cpp
kValidFreqMinHz
kValidFreqMaxHz
```

---

## Limitations

- ADC-based frequency measurement is simple and flexible, but not the most precise method.
- Accuracy decreases at higher frequencies because the ADC sample rate is limited.
- Best suited for low to mid frequency ranges (depending on waveform quality and amplitude).

---

## Images

### Schematic (PDF)

The schematic is stored as a **PDF** in the `Docs` branch:

- [Open Schematic PDF](https://github.com/X105GHM/Frequenzy_to_AnalogU/blob/Docs/Schematic_FtoA_2026-02-22.pdf)

### Perfboard / Build

![Perfboard Build](https://raw.githubusercontent.com/X105GHM/Frequenzy_to_AnalogU/Docs/FtoA_board.jpg)

---

## Oscillograms (Measured Signals)

This section shows oscilloscope screenshots from the real hardware.
`CH1` Output
`CH2` Input

### 150 Hz

![Oscillogram 150 Hz](https://raw.githubusercontent.com/X105GHM/Frequenzy_to_AnalogU/Docs/FtoA_150Hz.jpg)

### 240 Hz

![Oscillogram 240 Hz](https://raw.githubusercontent.com/X105GHM/Frequenzy_to_AnalogU/Docs/FtoA_240Hz.jpg)

### 500 Hz

![Oscillogram 500 Hz](https://raw.githubusercontent.com/X105GHM/Frequenzy_to_AnalogU/Docs/FtoA_500Hz.jpg)

---

## Author

Built for an **ATtiny85** using **PlatformIO**, **avr-libc**, and modern **C++17**.
