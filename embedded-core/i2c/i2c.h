#ifndef I2C_H
#define I2C_H

#ifdef UNIT_TEST

#include "mocks/i2c_master.h"
#include "mocks/esp_err.h"

#else

#include "driver/i2c_master.h"
#include "esp_err.h"

#endif

esp_err_t i2c_init(void);
i2c_master_bus_handle_t i2c_get_bus(void);

#ifdef UNIT_TEST

void i2c_reset(void);

#endif

#endif