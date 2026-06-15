#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

/* Mock types */

typedef void *i2c_master_bus_handle_t;

typedef struct
{
    int clk_source;
    int i2c_port;
    int scl_io_num;
    int sda_io_num;
    int glitch_ignore_cnt;

    struct
    {
        bool enable_internal_pullup;
    } flags;

} i2c_master_bus_config_t;

/* Mock constants */

#define I2C_CLK_SRC_DEFAULT 0
#define I2C_NUM_0           0

/* Mock function */

esp_err_t i2c_new_master_bus(
    const i2c_master_bus_config_t *config,
    i2c_master_bus_handle_t *bus_handle);

#endif