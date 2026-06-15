#include "gui_backlight.h"
#include "i2c/i2c.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

/**
 * @brief Handle to the backlight I2C device.
 *
 * Initialized during backlight_init() and used for
 * subsequent brightness updates.
 */
static i2c_master_dev_handle_t backlight_dev = NULL;

void set_brightness(uint8_t percent)
{
    if (backlight_dev == NULL)
        return;

    if (percent > BACKLIGHT_MAX_PCT)
        percent = BACKLIGHT_MAX_PCT;

    if (percent < BACKLIGHT_MIN_PCT)
        percent = BACKLIGHT_MIN_PCT;

    uint8_t level = (uint8_t)((100 - percent) * (255.0f / 100.0f));
    uint8_t buf[] = {BACKLIGHT_PWM_REG, level};
    i2c_master_transmit(backlight_dev, buf, sizeof(buf), -1);
}

void backlight_init(void)
{
    i2c_master_bus_handle_t bus_handle = i2c_get_bus();

    i2c_device_config_t dev_config = 
    {
        .dev_addr_length    = I2C_ADDR_BIT_LEN_7,
        .device_address     = BACKLIGHT_I2C_ADDR,
        .scl_speed_hz       = 400000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &backlight_dev));

    // Set all IO expander pins to output mode (register 0x02)
    uint8_t cmd[2] = {0x02, 0xFF};
    ESP_ERROR_CHECK(i2c_master_transmit(backlight_dev, cmd, 2, 100));

    // Drive all pins HIGH: releases touch reset (IO1) and LCD reset (IO3)
    cmd[0] = 0x03; cmd[1] = 0xFF;
    ESP_ERROR_CHECK(i2c_master_transmit(backlight_dev, cmd, 2, 100));

    vTaskDelay(pdMS_TO_TICKS(50));  // allow GT911 to boot after reset release

    set_brightness(50);
}