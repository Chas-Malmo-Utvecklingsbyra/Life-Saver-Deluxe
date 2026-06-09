#include "gui_screensaver.h"
#include "lvgl.h"

#define LCD_H_RES                   1024
#define LCD_V_RES                   600

ScreensaverControl ss               = {0};
uint32_t screensaver_timeout_ms     = 60000;
bool screensaver_enabled            = true;

static lv_obj_t *ss_bouncer         = NULL;
static int32_t ss_vel_x             = 3;
static int32_t ss_vel_y             = 2;

extern const lv_image_dsc_t chas_logo_small;

static void on_screensaver_tick(lv_timer_t *timer)
{
    if (!ss.active || ss_bouncer == NULL) return;

    int32_t x = lv_obj_get_x(ss_bouncer);
    int32_t y = lv_obj_get_y(ss_bouncer);
    int32_t w = lv_obj_get_width(ss_bouncer);
    int32_t h = lv_obj_get_height(ss_bouncer);

    lv_obj_invalidate(ss_bouncer);

    x += ss_vel_x;
    y += ss_vel_y;

    if (x <= 0)
    {
        x = 0;
        ss_vel_x = -ss_vel_x;
    }
    else if (x + w >= LCD_H_RES)
    {
        x = LCD_H_RES - w;
        ss_vel_x = -ss_vel_x;
    }

    if (y <= 0)
    {
        y = 0;
        ss_vel_y = -ss_vel_y;
    }
    else if (y + h >= LCD_V_RES)
    {
        y = LCD_V_RES - h;
        ss_vel_y = -ss_vel_y;
    }

    lv_obj_set_pos(ss_bouncer, x, y);
}

void screensaver_build(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x282A36), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    ss_bouncer = lv_image_create(screen);
    lv_image_set_src(ss_bouncer, &chas_logo_small);
    lv_obj_set_pos(ss_bouncer, LCD_H_RES / 3, LCD_V_RES / 3);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_invalidate(screen);
}

void screensaver_enter(lv_obj_t *screen_screensaver, lv_obj_t *screen_main, lv_timer_t *sensor_update_timer)
{
    (void)screen_main;

    if (ss.active) return;

    ss.active = true;

    if (ss.anim_timer == NULL)
    {
        ss.anim_timer = lv_timer_create(on_screensaver_tick, 33, NULL);
    }
    else
    {
        lv_timer_resume(ss.anim_timer);
    }

    if (sensor_update_timer)
    {
        lv_timer_pause(sensor_update_timer);
    }

    lv_screen_load(screen_screensaver);
}

void screensaver_exit(lv_obj_t *screen_main, lv_timer_t *ui_update_timer)
{
    if (!ss.active) return;

    ss.active = false;

    if (ss.anim_timer)
    {
        lv_timer_pause(ss.anim_timer);
    }

    if (ui_update_timer)
    {
        lv_timer_resume(ui_update_timer);
    }

    lv_indev_reset(NULL, NULL);    
    lv_screen_load(screen_main);
}