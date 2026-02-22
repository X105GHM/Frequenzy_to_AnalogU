#pragma once
#include <stdint.h>
#include "config/config.hpp"

namespace app
{
    [[nodiscard]] inline uint8_t frequencyToPwm(uint16_t freqHz)
    {
        if (freqHz <= cfg::kMapFreqMinHz)
        {
            return 0;
        }
        if (freqHz >= cfg::kMapFreqMaxHz)
        {
            return 255;
        }

        const uint32_t num = static_cast<uint32_t>(freqHz - cfg::kMapFreqMinHz) * 255UL;
        const uint32_t den = static_cast<uint32_t>(cfg::kMapFreqMaxHz - cfg::kMapFreqMinHz);

        return static_cast<uint8_t>(num / den);
    }
} 