#include <stdio.h>
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
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#define LCD_H_RES 1024
#define LCD_V_RES 600
#define TAG "TOUCH"

static esp_lcd_touch_handle_t touch_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static lv_obj_t *screen_home     = NULL;
static lv_obj_t *screen_settings = NULL;
static lv_obj_t *screen_logs     = NULL;
static lv_obj_t *screen_about    = NULL;

static i2c_master_bus_config_t bus_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .scl_io_num = 9,
    .sda_io_num = 8,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};
static i2c_master_bus_handle_t bus_handle = NULL;

static esp_lcd_panel_io_i2c_config_t io_config = {
    .dev_addr = 0x5D,
    .scl_speed_hz = 400000,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags.disable_control_phase = 1,
};

static void lv_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    lv_display_flush_ready(disp);
}

static void nav_button_cb(lv_event_t *e)
{
    ESP_LOGW(TAG, "Touch shit");
    /* lv_obj_t *target_screen = lv_event_get_user_data(e);
    lv_screen_load_anim(target_screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false); */
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_err_t ret = esp_lcd_touch_read_data(touch_handle);
    if (ret != ESP_OK) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    esp_lcd_touch_point_data_t touch_data = {};
    uint8_t point_count = 0;
    if (esp_lcd_touch_get_data(touch_handle, &touch_data, &point_count, 1) == ESP_OK && point_count > 0) {
        data->point.x = touch_data.x;
        data->point.y = touch_data.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void backlight_init(void)
{
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x24,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev_handle;
    if (i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle) == ESP_OK) {
        uint8_t write_buf[] = {0x02, 0x01};
        if (i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), -1) != ESP_OK) {
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
            1, 2, 42, 41, 40
        },
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
        LV_DISPLAY_RENDER_MODE_DIRECT
    );
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
}

static void create_sidebar(lv_obj_t *parent)
{
    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_set_size(sidebar, 200, 600);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(0x6272A4), 0);
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(sidebar, 15, 0);

    lv_obj_t *title = lv_label_create(sidebar);
    lv_label_set_text(title, "Life Saver Deluxe");
    lv_obj_set_style_text_color(title, lv_color_hex(0xF8F8F2), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);

    const char *button_names[]  = {"Home", "Settings", "Logs", "About us"};
    lv_obj_t   *target_screens[] = {screen_home, screen_settings, screen_logs, screen_about};

    for (int i = 0; i < 4; i++) {
        lv_obj_t *button = lv_button_create(sidebar);
        lv_obj_set_width(button, lv_pct(100));
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, button_names[i]);
        lv_obj_set_style_bg_color(button, lv_color_hex(0xFFB86C), 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(0xF1FA8C), LV_STATE_PRESSED);
        lv_obj_add_event_cb(button, nav_button_cb, LV_EVENT_CLICKED, target_screens[i]);
    }
}

static lv_obj_t * create_sensor(lv_obj_t *parent, const char *name, bool open)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_height(cont, 40);
    lv_obj_set_width(cont, LV_PCT(60));
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(cont, 10, 0);

    lv_obj_t *cont_label = lv_label_create(cont);
    lv_label_set_text(cont_label, name);

    lv_obj_t *state = lv_label_create(cont);
    if (open) {
        lv_label_set_text(state, "OPEN");
        lv_obj_set_style_text_color(state, lv_color_hex(0xFF0000), 0);
    } else {
        lv_label_set_text(state, "CLOSED");
        lv_obj_set_style_text_color(state, lv_color_hex(0x00FF00), 0);
    }
    lv_obj_align(state, LV_ALIGN_RIGHT_MID, -7, 0);
    return cont;
}

void create_security_ui(void)
{
    // Create all screens
    screen_home     = lv_obj_create(NULL);
    screen_settings = lv_obj_create(NULL);
    screen_logs     = lv_obj_create(NULL);
    screen_about    = lv_obj_create(NULL);

    // --- Home screen ---
    lv_obj_set_style_bg_color(screen_home, lv_color_hex(0xFF79C6), 0);
    lv_obj_t *home_main = lv_obj_create(screen_home);
    lv_obj_set_size(home_main, LCD_H_RES, LCD_V_RES);
    lv_obj_set_flex_flow(home_main, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(home_main, lv_color_hex(0x44475A), 0);
    lv_obj_set_style_pad_all(home_main, 0, 0);
    lv_obj_set_style_border_width(home_main, 0, 0);

    create_sidebar(home_main);

    lv_obj_t *content = lv_obj_create(home_main);
    lv_obj_set_size(content, 824, LCD_V_RES);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x282A36), 0);


    lv_obj_t *doors = lv_obj_create(content);
    lv_obj_set_flex_grow(doors, 1);
    lv_obj_set_flex_flow(doors, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(doors, lv_color_hex(0x6272A4), 0);

    lv_obj_t *windows = lv_obj_create(content);
    lv_obj_set_flex_grow(windows, 1);
    lv_obj_set_flex_flow(windows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(windows, lv_color_hex(0x6272A4), 0);

    create_sensor(doors, "Front Door", true);
    create_sensor(doors, "Back Door", false);
    create_sensor(windows, "Bedroom", true);
    create_sensor(windows, "Kitchen", false);

    // --- Settings screen (placeholder) ---
    lv_obj_set_style_bg_color(screen_settings, lv_color_hex(0x0000FF), 0);
    lv_obj_t *settings_main = lv_obj_create(screen_settings);
    lv_obj_set_size(settings_main, LCD_H_RES, LCD_V_RES);
    lv_obj_set_flex_flow(settings_main, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(settings_main, 0, 0);
    lv_obj_set_style_border_width(settings_main, 0, 0);
    create_sidebar(settings_main);
    lv_obj_t *settings_label = lv_label_create(settings_main);
    lv_label_set_text(settings_label, "Settings");
    lv_obj_set_style_text_color(settings_label, lv_color_hex(0x000000), 0);


    // --- Logs screen (placeholder) ---
    lv_obj_set_style_bg_color(screen_logs, lv_color_hex(0xFF0000), 0);
    lv_obj_t *logs_main = lv_obj_create(screen_logs);
    lv_obj_set_size(logs_main, LCD_H_RES, LCD_V_RES);
    lv_obj_set_flex_flow(logs_main, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(logs_main, 0, 0);
    lv_obj_set_style_border_width(logs_main, 0, 0);
    create_sidebar(logs_main);
    lv_obj_t *logs_label = lv_label_create(logs_main);
    lv_label_set_text(logs_label, "Logs");

    // --- About screen (placeholder) ---
    lv_obj_set_style_bg_color(screen_about, lv_color_hex(0x00F0F0), 0);
    lv_obj_t *about_main = lv_obj_create(screen_about);
    lv_obj_set_size(about_main, LCD_H_RES, LCD_V_RES);
    lv_obj_set_flex_flow(about_main, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(about_main, 0, 0);
    lv_obj_set_style_border_width(about_main, 0, 0);
    create_sidebar(about_main);
    lv_obj_t *about_label = lv_label_create(about_main);
    lv_label_set_text(about_label, "About us");

    lv_screen_load(screen_home);
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
        .levels = { .reset = 0, .interrupt = 0 },
        .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));

    // Register touch input AFTER touch_handle is valid
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    const esp_timer_create_args_t timer_args = {
        .callback = lv_tick_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t timer = NULL;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 1000);

    create_security_ui();

    while (1) {
        uint32_t delay_ms = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(delay_ms > 0 ? delay_ms : 1));
    }
}