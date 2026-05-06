#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

#define LCD_H_RES 1024
#define LCD_V_RES 600

#define TAG "TOUCH"

typedef struct
{
    uint8_t theme_index;
    lv_obj_t *home_main;
} theme_btn_data_t;

static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static lv_obj_t *screen_home = NULL;
static lv_obj_t *screen_settings = NULL;
static lv_obj_t *screen_logs = NULL;
static lv_obj_t *screen_about = NULL;
static lv_obj_t *wifi_status_label = NULL;

static lv_obj_t *log_textarea = NULL;

static uint8_t current_theme = 0;

static const i2c_master_bus_config_t bus_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .scl_io_num = 9,
    .sda_io_num = 8,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};
static i2c_master_bus_handle_t bus_handle = NULL;

static const esp_lcd_panel_io_i2c_config_t io_config = {
    .dev_addr = 0x5D,
    .scl_speed_hz = 400000,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags.disable_control_phase = 1,
};

static void create_security_ui(void);

/* =======================
    HARDWARE CALLBACKS:
==========================*/

static void lv_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_err_t ret = esp_lcd_touch_read_data(touch_handle);
    if (ret != ESP_OK)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    esp_lcd_touch_point_data_t touch_data = {};
    uint8_t point_count = 0;
    if (esp_lcd_touch_get_data(touch_handle, &touch_data, &point_count, 1) == ESP_OK && point_count > 0)
    {
        data->point.x = touch_data.x;
        data->point.y = touch_data.y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* =================
    UI CALLBACKS:
====================*/

static void nav_button_cb(lv_event_t *e)
{
    /* ESP_LOGW(TAG, "Touch shit"); */
    lv_obj_t *target_screen = lv_event_get_user_data(e);
    lv_screen_load_anim(target_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}

static void theme_btn_cb(lv_event_t *e)
{
    current_theme = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    create_security_ui();
}

/* ===================
    INIT FUNCTIONS:
======================*/

void backlight_init(void)
{
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x24,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;
    if (i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle) == ESP_OK)
    {
        uint8_t write_buf[] = {0x02, 0x01};
        if (i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), -1) != ESP_OK)
        {
            i2c_master_bus_rm_device(dev_handle);
        }
    }
}

void display_init(void)
{
    esp_lcd_rgb_panel_config_t config = {
        .data_width = 16,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .pclk_gpio_num = 7,
        .vsync_gpio_num = 3,
        .hsync_gpio_num = 46,
        .de_gpio_num = 5,
        .data_gpio_nums = {
            14, 38, 18, 17, 10,
            39, 0, 45, 48, 47, 21,
            1, 2, 42, 41, 40},
        .timings = {
            .pclk_hz = 12 * 1000 * 1000,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_back_porch = 140,
            .hsync_front_porch = 160,
            .hsync_pulse_width = 20,
            .vsync_back_porch = 20,
            .vsync_front_porch = 12,
            .vsync_pulse_width = 3,
        },
        .flags.fb_in_psram = true,
        .num_fbs = 2,
        .bounce_buffer_size_px = 1024 * 10,
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
}

void lvgl_port_init(void)
{
    lv_init();

    lv_display_t *disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    void *buf1 = NULL;
    void *buf2 = NULL;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));

    lv_display_set_buffers(
        disp,
        buf1,
        buf2,
        LCD_H_RES * LCD_V_RES * sizeof(lv_color_t),
        LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
}

/* =================================
    UI BUILDING HELPER FUNCTIONS:
====================================*/

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
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    wifi_status_label = lv_label_create(sidebar);
    lv_label_set_text(wifi_status_label, "Connecting..");
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(wifi_status_label, &lv_font_montserrat_14, 0);

    const char *button_names[] = {"Home", "Settings", "Logs", "About us"};
    lv_obj_t *target_screens[] = {screen_home, screen_settings, screen_logs, screen_about};

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
        lv_obj_add_event_cb(button, nav_button_cb, LV_EVENT_CLICKED, target_screens[i]);
    }
}

static lv_obj_t *create_sensor(lv_obj_t *parent, const char *name, bool open)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(card, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_hor(card, 16, 0);
    lv_obj_set_style_pad_ver(card, 0, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Colored status dot
    lv_obj_t *dot = lv_obj_create(card);
    lv_obj_set_size(dot, 12, 12);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_style_bg_color(dot, open ? lv_color_hex(0xFF5555) : lv_color_hex(0x50FA7B), 0);

    // Sensor name
    lv_obj_t *label = lv_label_create(card);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(label, 1);

    // Status text, right-aligned
    lv_obj_t *status = lv_label_create(card);
    lv_label_set_text(status, open ? "OPEN" : "CLOSED");
    lv_obj_set_style_text_color(status, open ? lv_color_hex(0xFF5555) : lv_color_hex(0x50FA7B), 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_14, 0);

    return card;
}

static void build_home_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_height(content, LCD_V_RES);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(content, lv_color_hex(t->bg), 0);
    lv_obj_set_style_pad_all(content, 0, 0);

    lv_obj_t *doors = lv_obj_create(content);
    lv_obj_set_flex_grow(doors, 1);
    lv_obj_set_height(doors, LCD_V_RES);
    lv_obj_set_flex_flow(doors, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(doors, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(doors, 0, 0);
    lv_obj_set_style_pad_all(doors, 10, 0);
    lv_obj_set_style_pad_gap(doors, 8, 0);

    lv_obj_t *doors_heading = lv_label_create(doors);
    lv_label_set_text(doors_heading, "Doors");
    lv_obj_set_style_text_color(doors_heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(doors_heading, &lv_font_montserrat_18, 0);

    lv_obj_t *windows = lv_obj_create(content);
    lv_obj_set_flex_grow(windows, 1);
    lv_obj_set_height(windows, LCD_V_RES);
    lv_obj_set_flex_flow(windows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(windows, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(windows, 0, 0);
    lv_obj_set_style_pad_all(windows, 10, 0);
    lv_obj_set_style_pad_gap(windows, 8, 0);

    lv_obj_t *windows_heading = lv_label_create(windows);
    lv_label_set_text(windows_heading, "Windows");
    lv_obj_set_style_text_color(windows_heading, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(windows_heading, &lv_font_montserrat_18, 0);

    create_sensor(doors, "Front Door", true);
    create_sensor(doors, "Back Door", false);
    create_sensor(windows, "Bedroom", true);
    create_sensor(windows, "Kitchen", false);
}

static void build_settings_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *section = lv_obj_create(parent);
    lv_obj_set_flex_grow(section, 1);
    lv_obj_set_height(section, LCD_V_RES);
    lv_obj_set_style_bg_color(section, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(section, 0, 0);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(section, 20, 0);
    lv_obj_set_style_pad_gap(section, 12, 0);

    lv_obj_t *heading = lv_label_create(section);
    lv_label_set_text(heading, "Color Theme");
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(heading, lv_color_hex(t->text), 0);

    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *btn = lv_button_create(section);
        lv_obj_set_width(btn, 260);
        lv_obj_set_style_bg_color(btn, lv_color_hex(themes[i].button), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(themes[i].button_pressed), LV_STATE_PRESSED);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, themes[i].name);
        lv_obj_set_style_text_color(lbl, lv_color_hex(themes[i].text), 0);
        lv_obj_add_event_cb(btn, theme_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    }
}

static void build_logs_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    log_textarea = lv_textarea_create(parent);
    lv_obj_set_flex_grow(log_textarea, 1);
    lv_obj_set_height(log_textarea, LCD_V_RES);
    lv_obj_set_style_bg_color(log_textarea, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_text_color(log_textarea, lv_color_hex(t->text), 0);
    lv_textarea_set_placeholder_text(log_textarea, "No logs yet...");
    lv_obj_set_style_border_width(log_textarea, 0, 0);
    lv_textarea_set_one_line(log_textarea, false);
}

static void build_about_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *section = lv_obj_create(parent);
    lv_obj_set_flex_grow(section, 1);
    lv_obj_set_height(section, LCD_V_RES);
    lv_obj_set_style_bg_color(section, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_border_width(section, 0, 0);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(section, 20, 0);
    lv_obj_set_style_pad_gap(section, 10, 0);

    lv_obj_t *heading = lv_label_create(section);
    lv_label_set_text(heading, "Life Saver Deluxe");
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(heading, lv_color_hex(t->text), 0);

    lv_obj_t *body = lv_label_create(section);
    lv_label_set_text(body,
                      "Version 1.0\n\n"
                      "A security monitoring system\n"
                      "for doors and windows.\n\n"
                      "Built with ESP32-S3 and LVGL.\n\n"
                      "Created by the wonderful team of CHAS Malmo Utvecklingsbyra\n\nContributors:\nEmilio 'The Wonderkid' Ganibegovic\nPar Lundh\nHenrik Westerlund\nLukas Stade\nIsa 'The Fixer' Shipshani.\n");
    lv_obj_set_style_text_color(body, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(body, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body, LV_PCT(100));
}

/* ==============================
    MAIN UI ENTRY POINT FUNCTION:
=================================*/

static void create_security_ui(void)
{
    const theme_t *t = &themes[current_theme];

    // Delete old screens if they exist (for theme switching)
    if (screen_home)
    {
        lv_obj_delete(screen_home);
        screen_home = NULL;
    }
    if (screen_settings)
    {
        lv_obj_delete(screen_settings);
        screen_settings = NULL;
    }
    if (screen_logs)
    {
        lv_obj_delete(screen_logs);
        screen_logs = NULL;
    }
    if (screen_about)
    {
        lv_obj_delete(screen_about);
        screen_about = NULL;
    }
    log_textarea = NULL;

    screen_home = lv_obj_create(NULL);
    screen_settings = lv_obj_create(NULL);
    screen_logs = lv_obj_create(NULL);
    screen_about = lv_obj_create(NULL);

#define MAKE_ROOT(scr, color) ({                           \
    lv_obj_t *_r = lv_obj_create(scr);                     \
    lv_obj_set_size(_r, LCD_H_RES, LCD_V_RES);             \
    lv_obj_set_flex_flow(_r, LV_FLEX_FLOW_ROW);            \
    lv_obj_set_style_bg_color(_r, lv_color_hex(color), 0); \
    lv_obj_set_style_pad_all(_r, 0, 0);                    \
    lv_obj_set_style_border_width(_r, 0, 0);               \
    lv_obj_set_pos(_r, 0, 0);                              \
    _r;                                                    \
})

    lv_obj_t *home_root = MAKE_ROOT(screen_home, t->bg);
    lv_obj_t *settings_root = MAKE_ROOT(screen_settings, t->bg);
    lv_obj_t *logs_root = MAKE_ROOT(screen_logs, t->bg);
    lv_obj_t *about_root = MAKE_ROOT(screen_about, t->bg);

#undef MAKE_ROOT

    create_sidebar(home_root);
    create_sidebar(settings_root);
    create_sidebar(logs_root);
    create_sidebar(about_root);

    build_home_content(home_root);
    build_settings_content(settings_root);
    build_logs_content(logs_root);
    build_about_content(about_root);

    lv_screen_load(screen_home);
}

/* ==============
    MAIN TASK:
=================*/
void GUI_Update_Network_Status(network_state_t state)
{
    if (wifi_status_label == NULL)
    {
        return;
    }

    switch(state)
    {
        case NETWORK_ONLINE:

            lv_label_set_text(wifi_status_label, "ONLINE");

            lv_obj_set_style_text_color(
                wifi_status_label,
                lv_color_hex(0x00FF00),
                0
            );

            break;

        case NETWORK_OFFLINE:

            lv_label_set_text(wifi_status_label, "OFFLINE MODE");

            lv_obj_set_style_text_color(
                wifi_status_label,
                lv_color_hex(0xFF0000),
                0
            );

            break;

        case NETWORK_CONNECTING:

            lv_label_set_text(wifi_status_label, "CONNECTING...");

            lv_obj_set_style_text_color(
                wifi_status_label,
                lv_color_hex(0xFFFF00),
                0
            );

            break;
    }
}

void lvgl_task(void *arg)
{
    backlight_init();
    display_init();
    lvgl_port_init();

    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle));

    esp_lcd_touch_config_t touch_config = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_42,
        .int_gpio_num = GPIO_NUM_4,
        .levels = {.reset = 0, .interrupt = 0},
        .flags = {.swap_xy = 0, .mirror_x = 0, .mirror_y = 0},
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    const esp_timer_create_args_t timer_args = {
        .callback = lv_tick_cb,
        .name = "lvgl_tick"};
    esp_timer_handle_t timer = NULL;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 1000);

    create_security_ui();

    while (1)
    {
        uint32_t delay_ms = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(delay_ms > 0 ? delay_ms : 1));
    }
}