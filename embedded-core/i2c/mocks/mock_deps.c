#include "mock_deps.h"

/* Mock return value */

esp_err_t mock_i2c_new_master_bus_result = ESP_OK;

/* Mock state */

int mock_i2c_new_master_bus_call_count = 0;

i2c_master_bus_handle_t mock_bus_handle =
    (i2c_master_bus_handle_t)0x1234;

/* Mock implementation */

esp_err_t i2c_new_master_bus(
    const i2c_master_bus_config_t *config,
    i2c_master_bus_handle_t *bus_handle)
{
    (void)config;

    mock_i2c_new_master_bus_call_count++;

    if (mock_i2c_new_master_bus_result == ESP_OK)
    {
        *bus_handle = mock_bus_handle;
    }

    return mock_i2c_new_master_bus_result;
}