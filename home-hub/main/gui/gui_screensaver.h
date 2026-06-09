#ifndef GUI_SCREENSAVER_H
#define GUI_SCREENSAVER_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

typedef struct
{
    bool active;
    lv_timer_t *anim_timer;
} ScreensaverControl;

extern ScreensaverControl ss;
extern uint32_t screensaver_timeout_ms;
extern bool screensaver_enabled;

void screensaver_build(lv_obj_t *screen);
void screensaver_enter(lv_obj_t *screen_screensaver, lv_obj_t *screen_main, lv_timer_t *sensor_update_timer);
void screensaver_exit(lv_obj_t *screen_main, lv_timer_t *ui_update_timer);

#endif