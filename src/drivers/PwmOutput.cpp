#include "drivers/PwmOutput.hpp"
#include "config/config.hpp"

#include <avr/io.h>

namespace drivers::pwm
{
    void init()
    {
        // PB0 (OC0A) als Ausgang
        DDRB |= _BV(cfg::kPwmPinBit);

        // Timer0: Fast PWM, non-inverting auf OC0A, Prescaler = 1
        // PWM-Frequenz = F_CPU / 256 = 31.25 kHz bei 8 MHz
        TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
        TCCR0B = _BV(CS00);

        OCR0A = 0;
    }

    void setDuty(uint8_t duty)
    {
        OCR0A = duty;
    }
} 