#ifndef GUI_THEMES_H
#define GUI_THEMES_H

#include <stdint.h>
#include "lvgl.h"

/**
 * @brief Theme definition used by the GUI.
 *
 * Contains all colors, fonts and image resources
 * required to render a complete application theme.
 */
typedef struct 
{
    const char *name;
    uint32_t bg;
    uint32_t sidebar;
    uint32_t button;
    uint32_t button_pressed;
    uint32_t content_bg;
    uint32_t sensor_bg;
    uint32_t text;
    uint32_t text_secondary;
    const lv_image_dsc_t *background_img;
    const lv_image_dsc_t *logo_img;
    const lv_font_t *font_large;
    const lv_font_t *font_normal;
    const lv_font_t *font_small;
} theme_t;

/**
 * @brief Available application themes.
 */
extern const theme_t themes[4];

/**
 * @brief Index of the currently active theme.
 */
extern uint8_t current_theme;

#endif