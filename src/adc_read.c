#include "adc_read.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

const uint8_t channel_length = 2;
uint8_t channel_array[2] = {18, 17};
uint16_t channels[2];
volatile bool flag;

/*
void read_temperature_sensor_blocking(uint16_t *target, uint16_t *vrefint) {
  adc_set_regular_sequence(ADC1, 2, channel_array);

  adc_start_conversion_regular(ADC1);
  while (!(adc_eoc(ADC1)))
    ;
  *target = adc_read_regular(ADC1);

  while (!(adc_eoc(ADC1)))
    ;
  *vrefint = adc_read_regular(ADC1);
}
*/

void read_temperature_sensor_blocking(uint16_t *target, uint16_t *vrefint) {
  flag = false;
  adc_start_conversion_regular(ADC1);

  while (!flag)
    ;  // Wait until DMA IRQ.
  *target = channels[0];
  *vrefint = channels[1];
}

void dma2_stream0_isr() {
  flag = true;

  if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_TCIF))
    dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
}

void temperature_dma_init(void) {
  rcc_periph_clock_enable(RCC_DMA2);
  nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
  dma_stream_reset(DMA2, DMA_STREAM0);
  dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_LOW);
  dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_16BIT);
  dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_16BIT);
  dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
  dma_enable_circular_mode(DMA2, DMA_STREAM0);
  dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
  dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t)&ADC1_DR);
  dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t)channels);
  dma_set_number_of_data(DMA2, DMA_STREAM0, channel_length);
  dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
  dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_0);
  dma_enable_stream(DMA2, DMA_STREAM0);
}

void temperature_init(void) {
  rcc_periph_clock_enable(RCC_ADC1);
  adc_power_off(ADC1);
  temperature_dma_init();
  adc_enable_scan_mode(ADC1);
  adc_set_single_conversion_mode(ADC1);
  adc_disable_external_trigger_regular(ADC1);
  adc_set_right_aligned(ADC1);
  /* We want to read the temperature sensor, so we have to enable it. */
  adc_enable_temperature_sensor();
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28CYC);
  adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
  adc_set_regular_sequence(ADC1, channel_length, (uint8_t *)channel_array);
  adc_set_dma_continue(ADC1);
  adc_enable_dma(ADC1);
  adc_power_on(ADC1);

  /* Wait for ADC starting up. */
  int i;
  for (i = 0; i < 800000; i++) __asm__("nop");
}
