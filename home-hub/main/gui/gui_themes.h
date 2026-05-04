#ifndef GUI_THEMES_H
#define GUI_THEMES_H

#include <stdint.h>

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
} theme_t;

extern const theme_t themes[4];

#endif