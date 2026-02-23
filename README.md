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
