#pragma once
#include <stdint.h>

namespace drivers::adc_freq
{
    void init();

    [[nodiscard]] bool readFrequencyHz(uint16_t &frequencyHz);
} 