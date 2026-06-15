#include "i2c.h"

#ifdef UNIT_TEST

#include "mocks/mock_deps.h"

#else

#include "driver/i2c_master.h"

#endif


static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t i2c_init(void)
{
    if (bus_handle != NULL)
        return ESP_OK;

    i2c_master_bus_config_t bus_config =
    {
        .clk_source                     = I2C_CLK_SRC_DEFAULT,
        .i2c_port                       = I2C_NUM_0,
        .scl_io_num                     = 9,
        .sda_io_num                     = 8,
        .glitch_ignore_cnt              = 7,
        .flags.enable_internal_pullup   = true,
    };

    return i2c_new_master_bus(&bus_config, &bus_handle);
}

i2c_master_bus_handle_t i2c_get_bus(void)
{
    return bus_handle;
}

#ifdef UNIT_TEST

void i2c_reset(void)
{
    bus_handle = NULL;
}

#endif