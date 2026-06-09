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
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "gui_backlight.h"
#include "gui_rename.h"
#include "gui_screensaver.h"
#include "gui_sensors.h"
#include "gui_sidebar.h"
#include "gui_tabs.h"
#include "gui_themes.h"

#include "internet/internet.h"
#include "../sensor/sensor.h"
#include "../sensor/sensor_settings.h"
#include "i2c/i2c.h"
#include "bme280/bme280.h"


#define TAG         "GUI"
#define LCD_H_RES   1024
#define LCD_V_RES   600


static esp_lcd_touch_handle_t touch_handle      = NULL;
static esp_lcd_panel_handle_t panel_handle      = NULL;
lv_display_t *lvgl_disp                         = NULL;
static lv_obj_t *screen_main                    = NULL;
static lv_obj_t *screen_screensaver             = NULL;
lv_obj_t *main_tabview                          = NULL;
uint8_t current_theme                           = 0;
static uint32_t last_input_time                 = 0;
static bool wifi_last_connected                 = false;
static bool wifi_first_update                   = true;
static lv_timer_t *ui_update_timer              = NULL;


static const esp_lcd_panel_io_i2c_config_t io_config = 
{
    .dev_addr                       = 0x5D,
    .scl_speed_hz                   = 400000,
    .control_phase_bytes            = 1,
    .dc_bit_offset                  = 0,
    .lcd_cmd_bits                   = 16,
    .flags.disable_control_phase    = 1,
};

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
            screensaver_exit(screen_main, ui_update_timer);
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

static void display_init(void)
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

static void lvgl_port_init(void)
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

void ui_rebuild_all(void)
{
    const theme_t *t = &themes[current_theme];

    close_rename_overlay();

    main_tabview        = NULL;
    wifi_status_label   = NULL;
    
    sensor_ui_reset();

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
    screensaver_build(screen_screensaver);

    screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_main, lv_color_hex(t->bg), 0);

    lv_obj_t *root = lv_obj_create(screen_main);
    lv_obj_set_size(root, LCD_H_RES, LCD_V_RES);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    ui_build_sidebar(root);

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
    lv_obj_t *tab_env       = lv_tabview_add_tab(main_tabview, "Environment");
    lv_obj_t *tab_settings  = lv_tabview_add_tab(main_tabview, "Settings");
    lv_obj_t *tab_about     = lv_tabview_add_tab(main_tabview, "About");

    lv_obj_set_style_pad_all(tab_home, 0, 0);
    lv_obj_set_style_pad_all(tab_settings, 0, 0);
    lv_obj_set_style_pad_all(tab_env, 0, 0);
    lv_obj_set_style_pad_all(tab_about, 0, 0);

    ui_build_tab_home(tab_home);
    ui_build_tab_env(tab_env);
    ui_build_tab_settings(tab_settings);
    ui_build_tab_about(tab_about);

    lv_screen_load(screen_main);
    lv_obj_update_layout(screen_main);
}

/* =======================
   UI EXTERNAL INTERFACE:
==========================*/

static void on_wifi_status_async(void *arg)
{
    if (wifi_status_label == NULL || ss.active) return;

    bool connected = Internet_Is_Connected();

    if (!wifi_first_update && connected == wifi_last_connected)
    {
        return;
    }

    wifi_first_update = false;
    wifi_last_connected = connected;

    const char *text = connected ? "ONLINE" : "OFFLINE MODE";
    uint32_t color = connected ? 0x50FA7B : 0xFF5555;
    
    lv_label_set_text(wifi_status_label, text);
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(color), 0);
}

void gui_update_network_status(void *arg)
{
    while (1)
    {
        lv_async_call(on_wifi_status_async, NULL);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void lvgl_task(void *arg)
{
    sensor_names_init();
    ESP_ERROR_CHECK(i2c_init());
    backlight_init();
    display_init();
    lvgl_port_init();

    vTaskDelay(pdMS_TO_TICKS(200)); // give screen some time to initialize

    i2c_master_bus_handle_t bus_handle = i2c_get_bus();

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

    ui_rebuild_all();

    ui_update_timer = lv_timer_create(on_ui_poll_timer, 250, NULL);
    last_input_time = lv_tick_get();

    xTaskCreate(
        gui_update_network_status,
        "GUIUpdateNetworkStatus",
        4096,
        NULL,
        8,
        NULL
    );

    while (1)
    {
        uint32_t now = lv_tick_get();

        if (screensaver_enabled && !ss.active && (now - last_input_time) >= screensaver_timeout_ms)
        {
            screensaver_enter(screen_screensaver, screen_main, ui_update_timer);
        }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}