#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "drivers/PwmOutput.hpp"
#include "drivers/AdcFrequencyMeter.hpp"
#include "app/FrequencyToPwmMapper.hpp"

int main()
{
  cli();

  drivers::pwm::init();
  drivers::adc_freq::init();

  sei();

  while (true)
  {
    uint16_t frequencyHz = 0;

    if (drivers::adc_freq::readFrequencyHz(frequencyHz))
    {
      const uint8_t duty = app::frequencyToPwm(frequencyHz);
      drivers::pwm::setDuty(duty);
    }
    else
    {
      drivers::pwm::setDuty(0); // Kein Signal -> 0V nach RC
    }
  }
}