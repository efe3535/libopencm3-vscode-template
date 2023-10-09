#include <stdint.h>

void temperature_init(void);
void temperature_dma_init(void);
void read_temperature_sensor_blocking(uint16_t *target, uint16_t *vrefint);