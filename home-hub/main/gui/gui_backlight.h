#ifndef GUI_BACKLIGHT_H
#define GUI_BACKLIGHT_H

#include <stdint.h>

/** I2C address of the backlight controller. */
#define BACKLIGHT_I2C_ADDR      0x24

/** PWM register used for brightness control. */
#define BACKLIGHT_PWM_REG       0x05

/** Minimum allowed brightness percentage. */
#define BACKLIGHT_MIN_PCT       10

/** Maximum allowed brightness percentage. */
#define BACKLIGHT_MAX_PCT       97

/**
 * @brief Sets the display brightness.
 *
 * Converts a percentage value into the PWM value required
 * by the backlight controller and transmits it over I2C.
 *
 * Values outside the supported range are clamped to the
 * configured minimum and maximum limits.
 *
 * @param percent Brightness percentage (0-100).
 *
 * @note Values are automatically clamped to valid limits.
 */
void set_brightness(uint8_t percent);

/**
 * @brief Initializes the LCD backlight controller.
 *
 * Configures the I2C backlight device, sets the required
 * IO expander registers, releases display and touch reset
 * lines, and applies a default brightness level.
 *
 * @note Must be called after I2C has been initialized.
 */
void backlight_init(void);

#endif