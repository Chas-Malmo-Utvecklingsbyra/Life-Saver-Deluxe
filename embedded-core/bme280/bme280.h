#ifndef BME280_H
#define BME280_H

#include "esp_err.h"

typedef struct
{
	int32_t T;
	uint32_t P;
	uint32_t H;
} bme280_meas_t;

esp_err_t bme280_work();

#endif