#ifndef I2C_H
#define I2C_H

#include "driver/i2c_master.h"
#include "esp_err.h"

esp_err_t i2c_init(void);
i2c_master_bus_handle_t i2c_get_bus(void);

#endif