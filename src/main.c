#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "adc_read.h"

int main(void) {
  temperature_init();
  while (1) {
    uint16_t temperature, vref,
        adcMaxVoltage;  // vref is around 1,21 volts (look at the datasheet)
    read_temperature_sensor_blocking(&temperature, &vref);
    /* adcMaxVoltage = vref; */
    // TODO: CALCULATE REFERENCE VOLTAGE.
  }

  return 0;
}