#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_heap_caps.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "gui_themes.h"
#include "internet/internet.h"
#include "../sensor/sensor.h"
#include "../sensor/sensor_settings.h"
#include "i2c/i2c.h"
#include "bme280/bme280.h"

/* =====================
        CONSTANTS:
======================*/

#define TAG                     "GUI"
#define LCD_H_RES               1024
#define LCD_V_RES               600
#define BACKLIGHT_I2C_ADDR      0x24
#define BACKLIGHT_PWM_REG       0x05
#define BACKLIGHT_MIN_PCT       10
#define BACKLIGHT_MAX_PCT       97
#define SENSOR_NAME_MAX         31

#define TAB_HOME                0
#define TAB_ENV                 1
#define TAB_SETTINGS            2
#define TAB_ABOUT               3

/* =====================
    STATIC VARIABLES:
======================*/

// Hardware handles
static esp_lcd_touch_handle_t touch_handle      = NULL;
static esp_lcd_panel_handle_t panel_handle      = NULL;
static i2c_master_bus_handle_t bus_handle       = NULL;
static i2c_master_dev_handle_t backlight_dev    = NULL;
static lv_display_t *lvgl_disp                  = NULL;

// Screen objects
static lv_obj_t *screen_main                    = NULL;
static lv_obj_t *screen_screensaver             = NULL;

// Tabview
static lv_obj_t *main_tabview                   = NULL;

// Shared widget references
static lv_obj_t *env_textarea                   = NULL;
static lv_obj_t *wifi_status_label              = NULL;

// Screensaver objects
typedef struct
{
    bool active;
    lv_timer_t *anim_timer;
} ScreensaverControl;

static ScreensaverControl ss                    = {0};
static uint32_t screensaver_timeout_ms          = 60000;    // 60s
static bool screensaver_enabled                 = true;
static lv_obj_t *ss_bouncer                     = NULL;
static int32_t ss_vel_x                         = 3;
static int32_t ss_vel_y                         = 2;
extern const lv_image_dsc_t chas_logo_small;

// State
static uint32_t last_input_time                 = 0;
static uint8_t current_theme                    = 0;
static bool wifi_last_connected                 = false;
static bool wifi_first_update                   = false;

// Keyboard/rename overlay
static lv_obj_t *rename_overlay                 = NULL;
static lv_obj_t *rename_ta                      = NULL;
static lv_obj_t *rename_kb                      = NULL;
static lv_obj_t *placement_dd                   = NULL;

// Sensor UI objects
typedef struct
{
    lv_obj_t *card;
    lv_obj_t *dot;
    lv_obj_t *name_label;
    lv_obj_t *status_label;
    Sensor   *sensor;
    char display_name[SENSOR_NAME_MAX + 1];
    bool last_open;
    bool last_has_data;
    bool initialized;
} SensorUi;

// ENV UI objects
typedef struct
{
    lv_obj_t *card;
    lv_obj_t *name_label;
    lv_obj_t *data_label;
    char data[20];
} BME280Ui;

static SensorUi *rename_target                  = NULL;
static SensorUi sensor_uis[MAX_SENSORS];
static size_t sensor_ui_count                   = 0;
static lv_timer_t *sensor_update_timer          = NULL;

static BME280Ui bme280_ui[3];
static lv_timer_t *env_update_timer             = NULL;

extern bme280_meas_t meas;
extern bool bme280_running;

/* =====================
    I2C CONFIGURATION:
======================*/

static const esp_lcd_panel_io_i2c_config_t io_config = 
{
    .dev_addr                       = 0x5D,
    .scl_speed_hz                   = 400000,
    .control_phase_bytes            = 1,
    .dc_bit_offset                  = 0,
    .lcd_cmd_bits                   = 16,
    .flags.disable_control_phase    = 1,
};

/* =====================
  FORWARD DECLARATIONS:
======================*/

static void create_security_ui(void);
static void update_sensor_ui_timer_cb(lv_timer_t *timer);
static void open_rename_overlay(SensorUi *ui);
static void close_rename_overlay(void);
static void screensaver_timer_cb(lv_timer_t *timer);

/* =====================
    BACKLIGHT CONTROL:
======================*/

static void set_brightness(uint8_t percent)
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

/* =======================
    SCREENSAVER CONTROL:
==========================*/

static void screensaver_enter(void)
{
    if (ss.active) return;

    ss.active = true;

    if (ss.anim_timer == NULL)
    {
        ss.anim_timer = lv_timer_create(screensaver_timer_cb, 33, NULL);
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

static void screensaver_exit(void)
{
    if (!ss.active) return;

    ss.active = false;

    if (ss.anim_timer)
    {
        lv_timer_pause(ss.anim_timer);
    }

    if (sensor_update_timer)
    {
        lv_timer_resume(sensor_update_timer);
    }

    lv_indev_reset(NULL, NULL);    
    lv_screen_load(screen_main);
}

/* =======================
    HARDWARE CALLBACKS:
==========================*/

static void lv_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_read_data(touch_handle);
    
    esp_lcd_touch_point_data_t touch_data;
    uint8_t point_count = 0;

    bool touched = (esp_lcd_touch_get_data(touch_handle, &touch_data, &point_count, 1) == ESP_OK) && (point_count > 0);
    
    if (touched)
    {
        last_input_time = lv_tick_get();

        if (ss.active)
        {
            screensaver_exit();

            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }

        data->point.x = touch_data.x;
        data->point.y = touch_data.y;
        data->state = LV_INDEV_STATE_PRESSED;
        return;
    }

    data->state = LV_INDEV_STATE_RELEASED;
}

/* =======================
   SENSOR STATE FUNCTION:
==========================*/

static void sensor_load_persistent_state(Sensor *sensor)
{
    SensorPlacement placement;

    bool found = sensor_placement_get(sensor->guid, &placement);

    if (found)
    {
        sensor->placement = placement;
    }
    else
    {
        sensor->placement = PLACEMENT_UNASSIGNED;
    }
}

/* =======================
    RENAMING KEYBOARD:
==========================*/

static void rename_kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY)
    {
        if (rename_target != NULL)
        {
            const char *typed = lv_textarea_get_text(rename_ta);

            if (typed != NULL && typed[0] != '\0')
            {
                strncpy(rename_target->display_name, typed, SENSOR_NAME_MAX);
                rename_target->display_name[SENSOR_NAME_MAX] = '\0';
                sensor_names_set(rename_target->sensor->guid, rename_target->display_name);
                lv_label_set_text(rename_target->name_label, rename_target->display_name);
            }

            uint16_t selected = lv_dropdown_get_selected(placement_dd);
            SensorPlacement placement = PLACEMENT_UNASSIGNED;

            switch (selected)
            {
                case 0:
                    placement = PLACEMENT_DOOR;
                    break;

                case 1:
                    placement = PLACEMENT_WINDOW;
                    break;

                case 2:
                default:
                    placement = PLACEMENT_UNASSIGNED;
                    break;
            }

            rename_target->sensor->placement = placement;
            sensor_placement_set(rename_target->sensor->guid, placement);
        }
        close_rename_overlay();
        
        create_security_ui();
    }
    else if (code == LV_EVENT_CANCEL)
    {
        close_rename_overlay();
    }
}

static void scrim_click_cb(lv_event_t *e)
{
    if (lv_event_get_target(e) == lv_event_get_current_target(e))
        close_rename_overlay();
}

static void open_rename_overlay(SensorUi *ui)
{
    close_rename_overlay();

    rename_target = ui;

    const theme_t *t = &themes[current_theme];

    lv_obj_t *layer = lv_display_get_layer_top(lvgl_disp);

    rename_overlay = lv_obj_create(layer);
    lv_obj_set_size(rename_overlay, LCD_H_RES, LCD_V_RES);
    lv_obj_set_pos(rename_overlay, 0, 0);
    lv_obj_set_style_bg_color(rename_overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(rename_overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(rename_overlay, 0, 0);
    lv_obj_set_style_pad_all(rename_overlay, 0, 0);
    lv_obj_set_style_radius(rename_overlay, 0, 0);
    lv_obj_add_event_cb(rename_overlay, scrim_click_cb, LV_EVENT_CLICKED, NULL);

    const int32_t PANEL_W = 700;
    const int32_t PANEL_H = 490;
    const int32_t PANEL_X = (LCD_H_RES - PANEL_W) / 2;
    const int32_t PANEL_Y = (LCD_V_RES - PANEL_H) / 2;

    lv_obj_t *panel = lv_obj_create(rename_overlay);
    lv_obj_set_size(panel, PANEL_W, PANEL_H);
    lv_obj_set_pos(panel, PANEL_X, PANEL_Y);
    lv_obj_set_style_bg_color(panel, lv_color_hex(t->sidebar), 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 16, 0);
    lv_obj_set_style_pad_all(panel, 16, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(panel, 10, 0);

    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);

    char heading_buf[SENSOR_NAME_MAX + 16];
    snprintf(heading_buf, sizeof(heading_buf), "Rename: %s", ui->display_name);
    lv_obj_t *heading = lv_label_create(panel);
    lv_label_set_text(heading, heading_buf);
    lv_obj_set_style_text_color(heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(heading, t->font_normal, 0);

    rename_ta = lv_textarea_create(panel);
    lv_obj_set_width(rename_ta, LV_PCT(100));
    lv_textarea_set_one_line(rename_ta, true);
    lv_textarea_set_max_length(rename_ta, SENSOR_NAME_MAX);
    lv_textarea_set_text(rename_ta, ui->display_name);
    lv_obj_set_style_bg_color(rename_ta, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_text_color(rename_ta, lv_color_hex(t->text), 0);
    lv_obj_set_style_border_color(rename_ta, lv_color_hex(t->button), 0);
    lv_obj_set_style_border_width(rename_ta, 2, 0);
    lv_obj_set_style_radius(rename_ta, 8, 0);

    placement_dd = lv_dropdown_create(panel);
    lv_dropdown_set_options(placement_dd, "Door\n""Window\n""Unassigned");
    uint16_t selected = 2;
    
    switch (ui->sensor->placement)
    {
        case PLACEMENT_DOOR:
            selected = 0;
            break;

        case PLACEMENT_WINDOW:
            selected = 1;
            break;

        case PLACEMENT_UNASSIGNED:
        default:
            selected = 2;
            break;
    }

    lv_dropdown_set_selected(placement_dd, selected);

    rename_kb = lv_keyboard_create(panel);
    lv_obj_set_width(rename_kb, LV_PCT(100));
    lv_obj_set_flex_grow(rename_kb, 1);
    lv_keyboard_set_textarea(rename_kb, rename_ta);
    lv_keyboard_set_mode(rename_kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_style_bg_color(rename_kb, lv_color_hex(t->sidebar), 0);
    lv_obj_set_style_bg_color(rename_kb, lv_color_hex(t->button), LV_PART_ITEMS);
    lv_obj_set_style_text_color(rename_kb, lv_color_hex(t->text), LV_PART_ITEMS);
    lv_obj_set_style_border_color(rename_kb, lv_color_hex(t->button_pressed), LV_PART_ITEMS);
    lv_obj_add_event_cb(rename_kb, rename_kb_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_invalidate(rename_overlay);
}

static void close_rename_overlay(void)
{
    if (rename_overlay != NULL)
    {
        lv_obj_delete(rename_overlay);
        rename_overlay = NULL;
    }

    rename_ta       = NULL;
    rename_kb       = NULL;
    rename_target   = NULL;
    placement_dd    = NULL;
}

/* =======================
        UI CALLBACKS:
==========================*/

static void nav_button_cb(lv_event_t *e)
{
    close_rename_overlay();

    uint32_t tab_index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    lv_tabview_set_active(main_tabview, tab_index, LV_ANIM_OFF);
}

static void theme_btn_cb(lv_event_t *e)
{
    current_theme = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    create_security_ui();
}

static void brightness_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    set_brightness((uint8_t)lv_slider_get_value(slider));
}

static void screensaver_timer_cb(lv_timer_t *timer)
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

static void screensaver_toggle_cb(lv_event_t *e)
{
    lv_obj_t *toggle = lv_event_get_target(e);
    screensaver_enabled = lv_obj_has_state(toggle, LV_STATE_CHECKED);
}

static void screensaver_timeout_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    screensaver_timeout_ms = (uint32_t)lv_slider_get_value(slider) * 1000;

    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    if (label)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%"PRIu32" s", (uint32_t)lv_slider_get_value(slider));
        lv_label_set_text(label, buf);
    }
}

static void open_rename_overlay_async(void *arg)
{
    open_rename_overlay((SensorUi *)arg);
}

static void sensor_card_tap_cb(lv_event_t *e)
{
    SensorUi *ui = (SensorUi *)lv_event_get_user_data(e);
    if (ui == NULL) return;

    if (lv_tabview_get_tab_active(main_tabview) != TAB_HOME) return;

    lv_async_call(open_rename_overlay_async, ui);
}

/* =======================
        INIT FUNCTIONS:
==========================*/

void backlight_init(void)
{
    bus_handle = i2c_get_bus();

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

void display_init(void)
{
    esp_lcd_rgb_panel_config_t config = 
    {
        .data_width         = 16,
        .clk_src            = LCD_CLK_SRC_DEFAULT,
        .pclk_gpio_num      = 7,
        .vsync_gpio_num     = 3,
        .hsync_gpio_num     = 46,
        .de_gpio_num        = 5,
        .data_gpio_nums     = 
        {
            14, 38, 18, 17, 10,
            39, 0, 45, 48, 47, 21,
            1, 2, 42, 41, 40
        },
        .timings = 
        {
            .pclk_hz            = 20 * 1000 * 1000,
            .h_res              = LCD_H_RES,
            .v_res              = LCD_V_RES,
            .hsync_back_porch   = 140,
            .hsync_front_porch  = 160,
            .hsync_pulse_width  = 20,
            .vsync_back_porch   = 20,
            .vsync_front_porch  = 12,
            .vsync_pulse_width  = 3,
        },
        .flags.fb_in_psram      = true,
        .num_fbs                = 2,
        .bounce_buffer_size_px  = LCD_H_RES * 10,
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
}

void lvgl_port_init(void)
{
    lv_init();

    lvgl_disp = lv_display_create(LCD_H_RES, LCD_V_RES);

    void *buf1 = NULL;
    void *buf2 = NULL;

    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));

    size_t buf_size = LCD_H_RES * LCD_V_RES * sizeof(lv_color_t);

    lv_display_set_buffers(lvgl_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_color_format(lvgl_disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvgl_disp, lvgl_flush_cb);
}

/* =======================
    UI BUILD HELPERS:
==========================*/

static void create_sidebar(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_set_size(sidebar, 200, LCD_V_RES);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(t->sidebar), 0);
    lv_obj_set_style_border_width(sidebar, 0, 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(sidebar, 12, 0);
    lv_obj_set_style_pad_gap(sidebar, 10, 0);

    lv_obj_t *title = lv_label_create(sidebar);
    lv_label_set_text(title, "Life Saver Deluxe");
    lv_obj_set_style_text_color(title, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(title, t->font_normal, 0);

    wifi_status_label = lv_label_create(sidebar);
    lv_label_set_text(wifi_status_label, "Connecting...");
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(wifi_status_label, t->font_small, 0);

    const char *button_names[] = 
    {
        "Home",
        "Environment",
        "Settings",
        "About us"
    };

    const uint32_t tab_pages[] = 
    {
        TAB_HOME,
        TAB_ENV,
        TAB_SETTINGS,
        TAB_ABOUT
    };

    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *button = lv_button_create(sidebar);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_set_style_bg_color(button, lv_color_hex(t->button), 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(t->button_pressed), LV_STATE_PRESSED);
        lv_obj_set_style_radius(button, 8, 0);

        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, button_names[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(t->text), 0);

        lv_obj_add_event_cb(button, nav_button_cb, LV_EVENT_PRESSED, (void*)(uintptr_t)tab_pages[i]);
    }
}

static lv_obj_t *create_sensor(lv_obj_t *parent, Sensor *sensor, const char *name)
{
    const theme_t *t = &themes[current_theme];

    bool has_data = sensor != NULL && sensor->data != NULL;
    bool open = has_data && *(bool *)sensor->data;

    uint32_t status_color = !has_data ? 0x888888 : (open ? 0xFF5555 : 0x50FA7B);

    char display_name[SENSOR_NAME_MAX + 1];

    bool have_saved = false;
    if (sensor != NULL && sensor->guid[0] != '\0')
    {
        have_saved = sensor_names_get(sensor->guid, display_name, sizeof(display_name));
    }

    if (!have_saved)
    {
        strncpy(display_name, name, SENSOR_NAME_MAX);
    }

    display_name[SENSOR_NAME_MAX] = '\0';

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(card, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_hor(card, 16, 0);
    lv_obj_set_style_pad_ver(card, 0, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *dot = lv_obj_create(card);
    lv_obj_set_size(dot, 12, 12);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_style_bg_color(dot, lv_color_hex(status_color), 0);

    lv_obj_t *label = lv_label_create(card);
    lv_label_set_text(label, display_name);
    lv_obj_set_style_text_color(label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(label, t->font_small, 0);
    lv_obj_set_flex_grow(label, 1);

    lv_obj_t *edit_hint = lv_label_create(card);
    lv_label_set_text(edit_hint, LV_SYMBOL_EDIT);
    lv_obj_set_style_text_color(edit_hint, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(edit_hint, t->font_small, 0);

    lv_obj_t *status = lv_label_create(card);
    lv_label_set_text(status, !has_data ? "UNKNOWN" : (open ? "OPEN" : "CLOSED"));
    lv_obj_set_style_text_color(status, lv_color_hex(status_color), 0);
    lv_obj_set_style_text_font(status, t->font_small, 0);

    if (sensor_ui_count >= MAX_SENSORS)
    {
        ESP_LOGE(TAG, "sensor_uis[] full - sensor card dropped");
        return NULL;
    }
    
    sensor_uis[sensor_ui_count].card = card;
    sensor_uis[sensor_ui_count].dot = dot;
    sensor_uis[sensor_ui_count].name_label = label;
    sensor_uis[sensor_ui_count].status_label = status;
    sensor_uis[sensor_ui_count].sensor = sensor;
    sensor_uis[sensor_ui_count].initialized = false;
    sensor_uis[sensor_ui_count].last_open = false;
    sensor_uis[sensor_ui_count].last_has_data = false;    

    memcpy(sensor_uis[sensor_ui_count].display_name, display_name, SENSOR_NAME_MAX);
    sensor_uis[sensor_ui_count].display_name[SENSOR_NAME_MAX] = '\0';

    lv_obj_add_event_cb(card, sensor_card_tap_cb, LV_EVENT_CLICKED, &sensor_uis[sensor_ui_count]);

    sensor_ui_count++;

    return card;
}

static lv_obj_t *create_settings_card(lv_obj_t *parent, const char *title)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_style_bg_color(card, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 15, 0);
    lv_obj_set_style_pad_gap(card, 10, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    
    lv_obj_t *heading = lv_label_create(card);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_color(heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(heading, t->font_normal, 0);

    return card;
}

static lv_obj_t *make_root(lv_obj_t *screen)
{
    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_set_size(root, LCD_H_RES, LCD_V_RES);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    return root;
}

/* =======================
      SCREEN CONTENT:
==========================*/

static void build_home_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    sensor_ui_count = 0;
    memset(sensor_uis, 0, sizeof(sensor_uis));

    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(content, lv_color_hex(t->bg), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(content, LV_DIR_NONE);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *top_row = lv_obj_create(content);
    lv_obj_set_width(top_row, LV_PCT(100));
    lv_obj_set_height(top_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(top_row, 1);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(top_row, 0, 0);
    lv_obj_set_style_pad_all(top_row, 0, 0);
    lv_obj_set_style_pad_gap(top_row, 10, 0);

    lv_obj_t *doors = lv_obj_create(top_row);
    lv_obj_set_flex_grow(doors, 1);
    lv_obj_set_height(doors, LV_PCT(100));
    lv_obj_set_flex_flow(doors, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(doors, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(doors, 0, 0);
    lv_obj_set_style_pad_all(doors, 10, 0);
    lv_obj_set_style_pad_gap(doors, 8, 0);

    lv_obj_t *doors_heading = lv_label_create(doors);
    lv_label_set_text(doors_heading, "Doors");
    lv_obj_set_style_text_color(doors_heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(doors_heading, t->font_normal, 0);

    lv_obj_t *windows = lv_obj_create(top_row);
    lv_obj_set_flex_grow(windows, 1);
    lv_obj_set_height(windows, LV_PCT(100));
    lv_obj_set_flex_flow(windows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(windows, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(windows, 0, 0);
    lv_obj_set_style_pad_all(windows, 10, 0);
    lv_obj_set_style_pad_gap(windows, 8, 0);

    lv_obj_t *windows_heading = lv_label_create(windows);
    lv_label_set_text(windows_heading, "Windows");
    lv_obj_set_style_text_color(windows_heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(windows_heading, t->font_normal, 0);

    lv_obj_t *unassigned = lv_obj_create(content);
    lv_obj_set_width(unassigned, LV_PCT(100));
    lv_obj_set_height(unassigned, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(unassigned, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(unassigned, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(unassigned, 0, 0);
    lv_obj_set_style_pad_all(unassigned, 10, 0);
    lv_obj_set_style_pad_gap(unassigned, 8, 0);

    lv_obj_t *unassigned_heading = lv_label_create(unassigned);
    lv_label_set_text(unassigned_heading, "Unassigned");
    lv_obj_set_style_text_color(unassigned_heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(unassigned_heading, t->font_normal, 0);


    size_t real_count = 0;

    Sensor *all_sensors = Sensor_Get_All();

    if (all_sensors == NULL)
    {
        ESP_LOGW(TAG, "Sensor_Get_All returned NULL");
        return;
    }

    for (size_t i = 0; i < MAX_SENSORS; i++)
    {
        Sensor *sensor = &all_sensors[i];

        ESP_LOGW(TAG, "%s %d", sensor->guid, sensor->type);

        if (sensor->type == 1)
        {
            real_count++;

            char buffer[64];

            if (sensor->guid[0] != '\0')
            {
                snprintf(buffer, sizeof(buffer), "%s", sensor->guid);
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "Sensor (%zu)", real_count);
            }

            ESP_LOGW(
                TAG,
                "ADDING MAGNETIC SENSOR CARD: %s data=%p",
                buffer,
                sensor->data
            );

            sensor_load_persistent_state(sensor);

            lv_obj_t *target_panel = doors;

            switch (sensor->placement)
            {
                case PLACEMENT_DOOR:
                    target_panel = doors;
                    break;

                case PLACEMENT_WINDOW:
                    target_panel = windows;
                    break;

                case PLACEMENT_UNASSIGNED:
                default:
                    target_panel = unassigned;
                    break;
            }

            create_sensor(target_panel, sensor, buffer);
        }
    }

    ESP_LOGW(TAG, "Total magnetic sensor cards added: %zu", real_count);
}

static void build_settings_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *section = lv_obj_create(parent);
    lv_obj_set_size(section, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(section, lv_color_hex(t->bg), 0);
    lv_obj_set_style_border_width(section, 0, 0);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(section, 10, 0);
    lv_obj_set_style_pad_gap(section, 10, 0);
    lv_obj_set_scrollbar_mode(section, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(section, LV_OBJ_FLAG_SCROLLABLE);
      
    lv_obj_t *theme_card = create_settings_card(section, "Color Themes");
    lv_obj_set_width(theme_card, LV_PCT(100));
    lv_obj_set_flex_grow(theme_card, 1);

    lv_obj_t *grid = lv_obj_create(theme_card);
    lv_obj_set_size(grid, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(grid, 10, 0);

    lv_obj_t *rows[2];
    for (int r = 0; r < 2; r++)
    {
        rows[r] = lv_obj_create(grid);
        lv_obj_set_width(rows[r], LV_PCT(100));
        lv_obj_set_height(rows[r], LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(rows[r], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(rows[r], 0, 0);
        lv_obj_set_style_pad_all(rows[r], 0, 0);
        lv_obj_set_flex_flow(rows[r], LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_gap(rows[r], 10, 0);
    }

    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *btn = lv_button_create(rows[i < 2 ? 0 : 1]);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_height(btn, 50);
        lv_obj_set_style_bg_color(btn, lv_color_hex(themes[i].button), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(themes[i].button_pressed), LV_STATE_PRESSED);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, themes[i].name);
        lv_obj_set_style_text_color(lbl, lv_color_hex(themes[i].text), 0);
        lv_obj_center(lbl);

        lv_obj_add_event_cb(btn, theme_btn_cb, LV_EVENT_PRESSED, (void *)(uintptr_t)i);
    }

    lv_obj_t *bottom_row = lv_obj_create(section);
    lv_obj_set_width(bottom_row, LV_PCT(100));
    lv_obj_set_flex_grow(bottom_row, 1);
    lv_obj_set_style_bg_opa(bottom_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bottom_row, 0, 0);
    lv_obj_set_style_pad_all(bottom_row, 0, 0);
    lv_obj_set_flex_flow(bottom_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(bottom_row, 10, 0);
    lv_obj_clear_flag(bottom_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *brightness_card = create_settings_card(bottom_row, "Brightness");
    lv_obj_set_flex_grow(brightness_card, 1);
    lv_obj_set_height(brightness_card, LV_PCT(100));

    lv_obj_t *slider = lv_slider_create(brightness_card);
    lv_obj_set_width(slider, LV_PCT(90));
    lv_slider_set_range(slider, BACKLIGHT_MIN_PCT, 100);
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *ss_card = create_settings_card(bottom_row, "Screensaver settings");
    lv_obj_set_flex_grow(ss_card, 1);
    lv_obj_set_height(ss_card, LV_PCT(100));
    
    lv_obj_t *toggle_row = lv_obj_create(ss_card);
    lv_obj_set_width(toggle_row, LV_PCT(100));
    lv_obj_set_height(toggle_row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(toggle_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toggle_row, 0, 0);
    lv_obj_set_style_pad_all(toggle_row, 0, 0);
    lv_obj_set_flex_flow(toggle_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggle_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *toggle_label = lv_label_create(toggle_row);
    lv_label_set_text(toggle_label, "Enabled");
    lv_obj_set_style_text_color(toggle_label, lv_color_hex(t->text), 0);

    lv_obj_t *sw = lv_switch_create(toggle_row);
    if (screensaver_enabled)
    {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(sw, screensaver_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);

    char timeout_buf[32];
    snprintf(timeout_buf, sizeof(timeout_buf), "Timeout: %"PRIu32" seconds", screensaver_timeout_ms / 1000);

    lv_obj_t *timeout_label = lv_label_create(ss_card);
    lv_label_set_text(timeout_label, timeout_buf);
    lv_obj_set_style_text_color(timeout_label, lv_color_hex(t->text), 0);

    lv_obj_t *ss_slider = lv_slider_create(ss_card);
    lv_obj_set_width(ss_slider, LV_PCT(90));
    lv_slider_set_range(ss_slider, 10, 300);
    lv_slider_set_value(ss_slider, screensaver_timeout_ms / 1000, LV_ANIM_OFF);
    lv_obj_add_event_cb(ss_slider, screensaver_timeout_slider_cb, LV_EVENT_VALUE_CHANGED, timeout_label);
}

static void build_env_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    env_textarea = lv_textarea_create(parent);
    lv_obj_set_size(env_textarea, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(env_textarea, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_text_color(env_textarea, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(env_textarea, t->font_normal, 0);
    lv_obj_set_style_border_width(env_textarea, 0, 0);

    lv_obj_t *temperature = lv_obj_create(parent);
    lv_obj_set_size(temperature, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(temperature, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(temperature, 0, 0);
    lv_obj_set_style_radius(temperature, 12, 0);
    lv_obj_set_style_pad_hor(temperature, 16, 0);
    lv_obj_set_style_pad_ver(temperature, 0, 0);
    lv_obj_set_flex_flow(temperature, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temperature, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(temperature, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *temperature_label = lv_label_create(temperature);
    lv_label_set_text(temperature_label, "Temp");
    lv_obj_set_style_text_color(temperature_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(temperature_label, t->font_small, 0);
    lv_obj_set_flex_grow(temperature_label, 1);

    lv_obj_t *temperature_data = lv_label_create(temperature);
    lv_label_set_text(temperature_data, "TestHum");
    lv_obj_set_style_text_color(temperature_data, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(temperature_data, t->font_small, 0);
    lv_obj_set_flex_grow(temperature_data, 1);

    lv_obj_t *pressure = lv_obj_create(parent);
    lv_obj_set_size(pressure, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(pressure, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(pressure, 0, 0);
    lv_obj_set_style_radius(pressure, 12, 0);
    lv_obj_set_style_pad_hor(pressure, 16, 0);
    lv_obj_set_style_pad_ver(pressure, 0, 0);
    lv_obj_set_flex_flow(pressure, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pressure, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(pressure, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *pressure_label = lv_label_create(pressure);
    lv_label_set_text(pressure_label, "Press");
    lv_obj_set_style_text_color(pressure_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(pressure_label, t->font_small, 0);
    lv_obj_set_flex_grow(pressure_label, 1);

    lv_obj_t *pressure_data = lv_label_create(pressure);
    lv_label_set_text(pressure_data, "TestHum");
    lv_obj_set_style_text_color(pressure_data, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(pressure_data, t->font_small, 0);
    lv_obj_set_flex_grow(pressure_data, 1);

    lv_obj_t *humidity = lv_obj_create(parent);
    lv_obj_set_size(humidity, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(humidity, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(humidity, 0, 0);
    lv_obj_set_style_radius(humidity, 12, 0);
    lv_obj_set_style_pad_hor(humidity, 16, 0);
    lv_obj_set_style_pad_ver(humidity, 0, 0);
    lv_obj_set_flex_flow(humidity, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(humidity, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(humidity, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *humidity_label = lv_label_create(humidity);
    lv_label_set_text(humidity_label, "Hum");
    lv_obj_set_style_text_color(humidity_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(humidity_label, t->font_small, 0);
    lv_obj_set_flex_grow(humidity_label, 1);

    lv_obj_t *humidity_data = lv_label_create(humidity);
    lv_label_set_text(humidity_data, "TestHum");
    lv_obj_set_style_text_color(humidity_data, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(humidity_data, t->font_small, 0);
    lv_obj_set_flex_grow(humidity_data, 1);

    bme280_ui[0].card = temperature;
    bme280_ui[0].name_label = temperature_label;
    bme280_ui[0].data_label = temperature_data;

    // env_update_timer = lv_timer_create(update_sensor_ui_timer_cb, 250, NULL);

    // char degc[20] = {0};
    // snprintf(degc, sizeof(degc), "%lu.%lu DegC", meas.T / 100, meas.T % 100);
    // lv_textarea_set_placeholder_text(env_textarea, (const char*)degc);
    // lv_textarea_set_placeholder_text(env_textarea, "No environment sensor detected");
    // lv_textarea_set_one_line(env_textarea, false);
}

static void build_about_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *section = lv_obj_create(parent);
    lv_obj_set_size(section, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(section, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(section, 0, 0);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(section, 20, 0);
    lv_obj_set_style_pad_gap(section, 10, 0);

    lv_obj_t *heading = lv_label_create(section);
    lv_label_set_text(heading, "Life Saver Deluxe");
    lv_obj_set_style_text_font(heading, t->font_normal, 0);
    lv_obj_set_style_text_color(heading, lv_color_hex(t->text), 0);

    lv_obj_t *body = lv_label_create(section);
    lv_label_set_text(body,
        "A security monitoring system\n"
        "for doors and windows.\n\n"
        "Built with ESP32-S3 and LVGL.\n\n"
        "Created by the wonderfully talented team of CHAS Malmo Utvecklingsbyra\n\n\n"
        "Contributors:\n"
        "============\n"
        "Emilio Ganibegovic\n"
        "Henrik Westerlund\n"
        "Isa Shipshani\n"
        "Lukas Stade\n"
        "Par Lundh\n"
    );

    lv_obj_set_style_text_color(body, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(body, t->font_small, 0);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body, LV_PCT(100));
}

/* =======================
      MAIN UI BUILDER:
==========================*/

static void create_security_ui(void)
{
    const theme_t *t = &themes[current_theme];

    close_rename_overlay();

    ss_bouncer          = NULL;
    main_tabview        = NULL;
    env_textarea        = NULL;
    wifi_status_label   = NULL;
    sensor_ui_count     = 0;
    memset(sensor_uis, 0, sizeof(sensor_uis));

    if (screen_main)
    {
        lv_obj_delete(screen_main);
        screen_main = NULL;
    }

    if (screen_screensaver)
    {
        lv_obj_delete(screen_screensaver);
        screen_screensaver = NULL;
    }

    screen_screensaver = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_screensaver, lv_color_hex(0x282A36), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_screensaver, LV_OPA_COVER, LV_PART_MAIN);

    ss_bouncer = lv_image_create(screen_screensaver);
    lv_image_set_src(ss_bouncer, &chas_logo_small);
    lv_obj_set_pos(ss_bouncer, LCD_H_RES / 3, LCD_V_RES / 3);
    lv_obj_set_style_bg_opa(screen_screensaver, LV_OPA_COVER, 0);
    lv_obj_invalidate(screen_screensaver);

    screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_main, lv_color_hex(t->bg), 0);

    lv_obj_t *root = make_root(screen_main);

    create_sidebar(root);

    main_tabview = lv_tabview_create(root);
    lv_tabview_set_tab_bar_size(main_tabview, 0);
    lv_obj_set_flex_grow(main_tabview, 1);
    lv_obj_set_height(main_tabview, LV_PCT(100));
    lv_obj_set_style_bg_color(main_tabview, lv_color_hex(t->bg), 0);
    lv_obj_set_style_border_width(main_tabview, 0, 0);
    lv_obj_set_style_pad_all(main_tabview, 0, 0);
    lv_obj_clear_flag(lv_tabview_get_content(main_tabview), LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *tab_content = lv_tabview_get_content(main_tabview);
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tab_content, lv_color_hex(t->bg), 0);
    lv_obj_set_style_bg_opa(tab_content, LV_OPA_COVER, 0);

    lv_obj_t *tab_home      = lv_tabview_add_tab(main_tabview, "Home");
    lv_obj_t *tab_env      = lv_tabview_add_tab(main_tabview, "Environment");
    lv_obj_t *tab_settings  = lv_tabview_add_tab(main_tabview, "Settings");
    lv_obj_t *tab_about     = lv_tabview_add_tab(main_tabview, "About");

    lv_obj_set_style_pad_all(tab_home, 0, 0);
    lv_obj_set_style_pad_all(tab_settings, 0, 0);
    lv_obj_set_style_pad_all(tab_env, 0, 0);
    lv_obj_set_style_pad_all(tab_about, 0, 0);

    build_home_content(tab_home);
    build_env_content(tab_env);
    build_settings_content(tab_settings);
    build_about_content(tab_about);

    lv_screen_load(screen_main);
    lv_obj_update_layout(screen_main);
}

/* =======================
   UI EXTERNAL INTERFACE:
==========================*/

static void gui_update_network_status_async(void *arg)
{
    if (wifi_status_label == NULL || ss.active) return;

    bool connected = Internet_Is_Connected();

    if (!wifi_first_update && connected == wifi_last_connected)
    {
        return;
    }

    wifi_first_update = false;
    wifi_last_connected = connected;

    const char *text = NULL;
    uint32_t color = 0;

    if (connected)
    {
        text = "ONLINE";
        color = 0x50FA7B;
    }
    else
    {
        text = "OFFLINE MODE";
        color = 0xFF5555;
    }
    
    lv_label_set_text(wifi_status_label, text);
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(color), 0);
}

void GUI_Update_Network_Status(void *arg)
{
    while (1)
    {
        lv_async_call(gui_update_network_status_async, NULL);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void update_sensor_ui_timer_cb(lv_timer_t *timer)
{
    for (size_t i = 0; i < sensor_ui_count; i++)
    {
        SensorUi *ui = &sensor_uis[i];

        if (ui->sensor == NULL)
            continue;

        void *data_ptr = ui->sensor->data;
        bool has_data = (data_ptr != NULL);
        bool open = has_data && *(bool *)data_ptr;

        if (ui->initialized && ui->last_has_data == has_data && ui->last_open == open)
        {
            continue;
        }

        ui->initialized = true;
        ui->last_has_data = has_data;
        ui->last_open = open;

        uint32_t status_color = !has_data ? 0x888888 : (open ? 0xFF5555 : 0x50FA7B);

        lv_label_set_text(ui->status_label, !has_data ? "UNKNOWN" : (open ? "OPEN" : "CLOSED"));
        lv_obj_set_style_text_color(ui->status_label, lv_color_hex(status_color), 0);
        lv_obj_set_style_bg_color(ui->dot, lv_color_hex(status_color), 0);
    }
}

static void update_env_ui_timer_cb(lv_timer_t *timer)
{
    BME280Ui *temp_ui = &bme280_ui[0];
    BME280Ui *press_ui = &bme280_ui[1];
    BME280Ui *hum_ui = &bme280_ui[2];

    if (bme280_running == false)
    {
        lv_textarea_set_placeholder_text(env_textarea, "No enviroment sensor connected");
        return; // Maybe??
    }

    snprintf(temp_ui->data, sizeof(temp_ui->data), "%lu.%lu", meas.T / 100, meas.T % 100);
    snprintf(press_ui->data, sizeof(temp_ui->data), "%lu", meas.P / 256);
    snprintf(hum_ui->data, sizeof(temp_ui->data), "%lu", meas.H / 1024);
    
    // TODO Update stuff
    lv_label_set_text(temp_ui->data_label, temp_ui->data);
    lv_label_set_text(press_ui->data_label, press_ui->data);
    lv_label_set_text(hum_ui->data_label, hum_ui->data);
}

/* =======================
        MAIN TASK:
==========================*/

void lvgl_task(void *arg)
{
    sensor_names_init();
    ESP_ERROR_CHECK(i2c_init());
    backlight_init();
    display_init();
    lvgl_port_init();

    vTaskDelay(pdMS_TO_TICKS(200)); // give screen some time to initialize
    esp_lcd_panel_io_handle_t touch_io = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &touch_io));

    // GT911 config - only dev_addr is needed since the driver handles the rest internally
    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = io_config.dev_addr,
    };

    esp_lcd_touch_config_t touch_config =
        {
            .x_max = LCD_H_RES,
            .y_max = LCD_V_RES,
            .rst_gpio_num = -1,
            .int_gpio_num = GPIO_NUM_4,
            .levels =
                {
                    .reset = 0,
                    .interrupt = 0},
            .flags =
                {
                    .swap_xy = 0,
                    .mirror_x = 0,
                    .mirror_y = 0},
            .driver_data = &tp_gt911_config,
        };

    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(touch_io, &touch_config, &touch_handle));

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    const esp_timer_create_args_t tick_timer_args = 
    {
        .callback = lv_tick_cb,
        .name     = "lvgl_tick",
    };

    esp_timer_handle_t tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000));

    create_security_ui();

    sensor_update_timer = lv_timer_create(update_sensor_ui_timer_cb, 250, NULL);

    last_input_time = lv_tick_get();

    // xTaskCreate(
    //     GUI_Update_Network_Status,
    //     "GUIUpdateNetworkStatus",
    //     4096,
    //     NULL,
    //     8,
    //     NULL
    // );

    while (1)
    {
        uint32_t now = lv_tick_get();

        if (screensaver_enabled && !ss.active && (now - last_input_time) >= screensaver_timeout_ms)
        {
            screensaver_enter();
        }

        uint32_t delay_ms = lv_timer_handler();
        // ESP_LOGI(TAG, "delay=%lu", delay_ms);
        vTaskDelay(pdMS_TO_TICKS(delay_ms ? delay_ms : 1));
    }
}