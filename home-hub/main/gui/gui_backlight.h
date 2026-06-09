#ifndef GUI_BACKLIGHT_H
#define GUI_BACKLIGHT_H

#include <stdint.h>

#define BACKLIGHT_I2C_ADDR      0x24
#define BACKLIGHT_PWM_REG       0x05
#define BACKLIGHT_MIN_PCT       10
#define BACKLIGHT_MAX_PCT       97

void backlight_init(void);
void set_brightness(uint8_t percent);

#endif