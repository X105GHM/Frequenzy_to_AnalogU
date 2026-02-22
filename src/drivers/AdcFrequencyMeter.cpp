#include "drivers/AdcFrequencyMeter.hpp"
#include "config/config.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>

namespace
{
    volatile uint32_t gSampleCounter = 0;
    volatile uint32_t gLastRiseSample = 0;
    volatile uint32_t gLastPeriodSamples = 0;
    volatile bool gSignalHigh = false;

    // Kleiner IIR-Filter für stabilere Ausgabe (Q3-Fixpunkt)
    uint32_t gFilteredFreq_x8 = 0;
}

namespace drivers::adc_freq
{

    void init()
    {
        // PB2 / ADC1 als Eingang, Pullup aus
        DDRB &= ~_BV(PB2);
        PORTB &= ~_BV(PB2);

        // Digital Input auf ADC1 
        DIDR0 |= _BV(ADC1D);

        // ADMUX:
        // ADLAR = 1 (8-bit ADCH)
        // MUX = ADC1
        ADMUX = _BV(ADLAR) | (cfg::kAdcChannel & 0x0F);

        // ADCSRA:
        // ADEN  = ADC Enable
        // ADATE = Auto Trigger (Free Running)
        // ADIE  = ADC Interrupt Enable
        // ADPS2 + ADPS0 = Prescaler 32
        // ADSC  = Start Conversion
        ADCSRA = _BV(ADEN) |
                 _BV(ADATE) |
                 _BV(ADIE) |
                 _BV(ADPS2) | _BV(ADPS0) |
                 _BV(ADSC);

// Free-running trigger source
#ifdef ADCSRB
        ADCSRB = 0;
#endif
    }

    [[nodiscard]] bool readFrequencyHz(uint16_t &frequencyHz)
    {
        uint32_t sampleCounter = 0;
        uint32_t lastRise = 0;
        uint32_t periodSamples = 0;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            sampleCounter = gSampleCounter;
            lastRise = gLastRiseSample;
            periodSamples = gLastPeriodSamples;
        }

        const bool signalAlive = (sampleCounter - lastRise) < cfg::kNoSignalSamples;

        if (!signalAlive || periodSamples == 0)
        {
            gFilteredFreq_x8 = 0;
            frequencyHz = 0;
            return false;
        }

        const uint16_t rawHz = static_cast<uint16_t>(cfg::kAdcSampleRate / periodSamples);

        // IIR-Glättung
        const uint32_t target_x8 = static_cast<uint32_t>(rawHz) * 8UL;

        if (gFilteredFreq_x8 == 0)
        {
            gFilteredFreq_x8 = target_x8;
        }
        else
        {
            const int32_t diff = static_cast<int32_t>(target_x8) - static_cast<int32_t>(gFilteredFreq_x8);

            gFilteredFreq_x8 = static_cast<uint32_t>(static_cast<int32_t>(gFilteredFreq_x8) + diff / 4);
        }

        frequencyHz = static_cast<uint16_t>(gFilteredFreq_x8 / 8UL);
        return true;
    }
} 

ISR(ADC_vect)
{
    const uint8_t sample = ADCH; // 8-bit ADC Wert
    const uint32_t nowSample = ++gSampleCounter;

    const uint8_t highThreshold = static_cast<uint8_t>(cfg::kThreshold + cfg::kHysteresis);
    const uint8_t lowThreshold = static_cast<uint8_t>(cfg::kThreshold - cfg::kHysteresis);

    if (!gSignalHigh)
    {
        if (sample >= highThreshold)
        {
            gSignalHigh = true;

            const uint32_t period = nowSample - gLastRiseSample;
            gLastRiseSample = nowSample;

            if (period >= cfg::kMinPeriodSamples && period <= cfg::kMaxPeriodSamples)
            {
                gLastPeriodSamples = period;
            }
        }
    }
    else
    {
        if (sample <= lowThreshold)
        {
            gSignalHigh = false;
        }
    }
}