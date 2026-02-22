#pragma once
#include <stdint.h>

namespace drivers::pwm
{
    void init();
    void setDuty(uint8_t duty);
}