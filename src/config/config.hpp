#pragma once
#include <stdint.h>

namespace cfg
{
    inline constexpr uint8_t kAdcChannel = 1;
    inline constexpr uint8_t kPwmPinBit = 0;

    inline constexpr uint32_t kAdcPrescaler = 32;
    inline constexpr uint32_t kAdcSampleRate = (F_CPU / kAdcPrescaler) / 13; // ~19.2 kS/s

    inline constexpr uint8_t kThreshold = 128;
    inline constexpr uint8_t kHysteresis = 8;

    inline constexpr uint16_t kValidFreqMinHz = 20;
    inline constexpr uint16_t kValidFreqMaxHz = 5000;

    inline constexpr uint16_t kMapFreqMinHz = 150;           // darunter 0V
    inline constexpr uint16_t kMapFreqMaxHz = 600;         // darüber Vollaussteuerung

    inline constexpr uint16_t kNoSignalTimeoutMs = 200;

    inline constexpr uint32_t kMinPeriodSamples = (kAdcSampleRate / kValidFreqMaxHz);
    inline constexpr uint32_t kMaxPeriodSamples = (kAdcSampleRate / kValidFreqMinHz);
    inline constexpr uint32_t kNoSignalSamples = (kAdcSampleRate * kNoSignalTimeoutMs) / 1000UL;
}