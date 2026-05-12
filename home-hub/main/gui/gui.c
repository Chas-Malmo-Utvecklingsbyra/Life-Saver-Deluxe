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
#include "../sensor/sensor.h"

/* =====================
        CONSTANTS:
======================*/

#define TAG                     "GUI"
#define LCD_H_RES               1024
#define LCD_V_RES               600
#define SCREENSAVER_TIMEOUT_MS  30000
#define BACKLIGHT_I2C_ADDR      0x24
#define BACKLIGHT_PWM_REG       0x05
#define BACKLIGHT_MIN_PCT       10
#define BACKLIGHT_MAX_PCT       97

/* =====================
        ENUMS:
======================*/

typedef enum
{
    STATE_ACTIVE,
    STATE_SCREENSAVER
} ScreenState;

/* =====================
    STATIC VARIABLES:
======================*/

// Hardware handles
static esp_lcd_touch_handle_t touch_handle      = NULL;
static esp_lcd_panel_handle_t panel_handle      = NULL;
static i2c_master_bus_handle_t bus_handle       = NULL;
static i2c_master_dev_handle_t backlight_dev    = NULL;

// Screen objects
static lv_obj_t *screen_home                    = NULL;
static lv_obj_t *screen_settings                = NULL;
static lv_obj_t *screen_logs                    = NULL;
static lv_obj_t *screen_about                   = NULL;
static lv_obj_t *screen_screensaver             = NULL;
static lv_obj_t *log_textarea                   = NULL;
static lv_obj_t *wifi_status_labels[]           = {NULL, NULL, NULL, NULL};
static uint8_t wifi_label_count                 = 0;

// Screensaver objects
static bool screensaver_consuming_release       = false;
static lv_timer_t *screensaver_timer            = NULL;
static lv_obj_t *ss_bouncer                     = NULL;
static int32_t ss_vel_x                         = 3;
static int32_t ss_vel_y                         = 2;
extern const lv_image_dsc_t chas_logo_small;

// State
static ScreenState screensaver_state            = STATE_ACTIVE;
static uint32_t last_input_time                 = 0;
static uint8_t current_theme                    = 0;
static volatile bool vsync_happened             = false;

// Sensor UI objects
typedef struct
{
    lv_obj_t *card;
    lv_obj_t *dot;
    lv_obj_t *name_label;
    lv_obj_t *status_label;
    Sensor   *sensor;
} SensorUi;

static SensorUi sensor_uis[MAX_SENSORS];
static size_t sensor_ui_count = 0;

/* =====================
    I2C CONFIGURATION:
======================*/

static const i2c_master_bus_config_t bus_config = {
    .clk_source             = I2C_CLK_SRC_DEFAULT,
    .i2c_port               = I2C_NUM_0,
    .scl_io_num             = 9,
    .sda_io_num             = 8,
    .glitch_ignore_cnt      = 7,
    .flags.enable_internal_pullup = true,
};

static const esp_lcd_panel_io_i2c_config_t io_config = {
    .dev_addr               = 0x5D,
    .scl_speed_hz           = 400000,
    .control_phase_bytes    = 1,
    .dc_bit_offset          = 0,
    .lcd_cmd_bits           = 16,
    .flags.disable_control_phase = 1,
};

/* =====================
  FORWARD DECLARATIONS:
======================*/

static void create_security_ui(void);
static void update_sensor_ui_timer_cb(lv_timer_t *timer);

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
    HARDWARE CALLBACKS:
==========================*/

static void lv_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

static bool panel_vsync_cb(
    esp_lcd_panel_handle_t panel,
    const esp_lcd_rgb_panel_event_data_t *event_data,
    void *user_ctx
)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vsync_happened = true;
    return xHigherPriorityTaskWoken == pdTRUE;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if (lv_display_flush_is_last(disp))
    {
        while (!vsync_happened) {}
        vsync_happened = false;
    }

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

    bool touched = (esp_lcd_touch_get_data(touch_handle, &touch_data, &point_count, 1) == ESP_OK && point_count > 0);

    if (screensaver_consuming_release)
    {
        if (!touched)
            screensaver_consuming_release = false;

        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    if (touched)
    {
        if (screensaver_state == STATE_SCREENSAVER)
        {
            screensaver_state = STATE_ACTIVE;
            last_input_time = lv_tick_get();

            if (screensaver_timer != NULL)
            {
                lv_timer_pause(screensaver_timer);
            }

            lv_screen_load(screen_home);
            lv_obj_invalidate(screen_home);
            screensaver_consuming_release = true;
            
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }

        last_input_time = lv_tick_get();

        data->point.x = touch_data.x;
        data->point.y = touch_data.y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* =======================
        UI CALLBACKS:
==========================*/

static void nav_button_cb(lv_event_t *e)
{
    lv_obj_t *target_screen = lv_event_get_user_data(e);

    lv_screen_load_anim(target_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    lv_obj_invalidate(target_screen);
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
    if (ss_bouncer == NULL) return;

    int32_t x = lv_obj_get_x(ss_bouncer);
    int32_t y = lv_obj_get_y(ss_bouncer);
    int32_t w = lv_obj_get_width(ss_bouncer);
    int32_t h = lv_obj_get_height(ss_bouncer);

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

/* =======================
        INIT FUNCTIONS:
==========================*/

void backlight_init(void)
{
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length    = I2C_ADDR_BIT_LEN_7,
        .device_address     = BACKLIGHT_I2C_ADDR,
        .scl_speed_hz       = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &backlight_dev));

    set_brightness(100);
}

void display_init(void)
{
    esp_lcd_rgb_panel_config_t config = {
        .data_width         = 16,
        .clk_src            = LCD_CLK_SRC_DEFAULT,
        .pclk_gpio_num      = 7,
        .vsync_gpio_num     = 3,
        .hsync_gpio_num     = 46,
        .de_gpio_num        = 5,
        .data_gpio_nums     = {
            14, 38, 18, 17, 10,
            39, 0, 45, 48, 47, 21,
            1, 2, 42, 41, 40
        },
        .timings = {
            .pclk_hz            = 12 * 1000 * 1000,
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

    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = panel_vsync_cb,
    };

    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, NULL));
}

void lvgl_port_init(void)
{
    lv_init();

    lv_display_t *disp = lv_display_create(LCD_H_RES, LCD_V_RES);

    void *buf1 = NULL;
    void *buf2 = NULL;

    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));

    size_t buf_size = LCD_H_RES * LCD_V_RES * sizeof(lv_color_t);

    lv_display_set_buffers(disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
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
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);

    if (wifi_label_count < 4)
    {
        wifi_status_labels[wifi_label_count] = lv_label_create(sidebar);
        lv_label_set_text(wifi_status_labels[wifi_label_count], "Connecting...");
        lv_obj_set_style_text_color(wifi_status_labels[wifi_label_count], lv_color_hex(t->text), 0);
        lv_obj_set_style_text_font(wifi_status_labels[wifi_label_count], &lv_font_montserrat_14, 0);
        wifi_label_count++;
    }

    const char *button_names[] = {
        "Home",
        "Settings",
        "Logs",
        "About us"
    };

    lv_obj_t *target_screens[] = {
        screen_home,
        screen_settings,
        screen_logs,
        screen_about
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

        lv_obj_add_event_cb(button, nav_button_cb, LV_EVENT_PRESSED, target_screens[i]);
    }
}

static lv_obj_t *create_sensor(lv_obj_t *parent, Sensor *sensor, const char *name)
{
    const theme_t *t = &themes[current_theme];

    bool has_data = sensor != NULL && sensor->data != NULL;
    bool open = false;

    if (has_data)
        open = *(bool *)sensor->data;

    uint32_t status_color = !has_data ? 0x888888 : (open ? 0xFF5555 : 0x50FA7B);

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(90), 56);
    lv_obj_set_style_bg_color(card, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_pad_hor(card, 16, 0);
    lv_obj_set_style_pad_ver(card, 0, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *dot = lv_obj_create(card);
    lv_obj_set_size(dot, 12, 12);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_style_bg_color(dot, lv_color_hex(status_color), 0);

    lv_obj_t *label = lv_label_create(card);
    lv_label_set_text(label, name);
    lv_obj_set_style_text_color(label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(label, 1);

    lv_obj_t *status = lv_label_create(card);
    lv_label_set_text(status, !has_data ? "UNKNOWN" : (open ? "OPEN" : "CLOSED"));
    lv_obj_set_style_text_color(status, lv_color_hex(status_color), 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_14, 0);

    if (sensor_ui_count < MAX_SENSORS)
    {
        sensor_uis[sensor_ui_count].card = card;
        sensor_uis[sensor_ui_count].dot = dot;
        sensor_uis[sensor_ui_count].name_label = label;
        sensor_uis[sensor_ui_count].status_label = status;
        sensor_uis[sensor_ui_count].sensor = sensor;
        sensor_ui_count++;
    }

    return card;
}

static lv_obj_t *make_root(lv_obj_t *screen)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_set_size(root, LCD_H_RES, LCD_V_RES);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_color(root, lv_color_hex(t->bg), 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);

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
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_height(content, LCD_V_RES);
    lv_obj_set_style_bg_color(content, lv_color_hex(t->bg), 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW);

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

            create_sensor(doors, sensor, buffer);
        }
    }

    ESP_LOGW(TAG, "Total magnetic sensor cards added: %zu", real_count);
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

    lv_obj_t *theme_heading = lv_label_create(section);
    lv_label_set_text(theme_heading, "Color Theme");
    lv_obj_set_style_text_font(theme_heading, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(theme_heading, lv_color_hex(t->text), 0);

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

        lv_obj_add_event_cb(btn, theme_btn_cb, LV_EVENT_PRESSED, (void *)(uintptr_t)i);
    }

    lv_obj_t *bright_heading = lv_label_create(section);
    lv_label_set_text(bright_heading, "Brightness");
    lv_obj_set_style_text_font(bright_heading, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(bright_heading, lv_color_hex(t->text), 0);

    lv_obj_t *slider = lv_slider_create(section);
    lv_obj_set_width(slider, 260);
    lv_slider_set_range(slider, BACKLIGHT_MIN_PCT, 100);
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(t->button), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(t->button), LV_PART_KNOB);
    lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

static void build_logs_content(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    log_textarea = lv_textarea_create(parent);
    lv_obj_set_flex_grow(log_textarea, 1);
    lv_obj_set_height(log_textarea, LCD_V_RES);
    lv_obj_set_style_bg_color(log_textarea, lv_color_hex(t->content_bg), 0);
    lv_obj_set_style_text_color(log_textarea, lv_color_hex(t->text), 0);
    lv_obj_set_style_border_width(log_textarea, 0, 0);
    lv_textarea_set_placeholder_text(log_textarea, "No logs yet...");
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
        "Version 1.1\n\n"
        "A security monitoring system\n"
        "for doors and windows.\n\n"
        "Built with ESP32-S3 and LVGL.\n\n"
        "Created by the wonderful team of CHAS Malmo Utvecklingsbyra\n\n\n"
        "Contributors:\n"
        "============\n"
        "Emilio 'The Wonderkid' Ganibegovic\n"
        "Henrik Westerlund\n"
        "Isa 'The Fixer' Shipshani.\n"
        "Lukas Stade\n"
        "Par Lundh\n"
    );

    lv_obj_set_style_text_color(body, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(body, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body, LV_PCT(100));
}

/* =======================
      MAIN UI BUILDER:
==========================*/

static void create_security_ui(void)
{
    const theme_t *t = &themes[current_theme];

    screensaver_consuming_release = false;
    screensaver_state = STATE_ACTIVE;

    if (screensaver_timer != NULL)
    {
        lv_timer_pause(screensaver_timer);
        screensaver_timer = NULL;
    }
    ss_bouncer = NULL;

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

    if (screen_screensaver)
    {
        lv_obj_delete(screen_screensaver);
        screen_screensaver = NULL;
    }

    log_textarea = NULL;

    memset(wifi_status_labels, 0, sizeof(wifi_status_labels));
    wifi_label_count = 0;

    sensor_ui_count = 0;
    memset(sensor_uis, 0, sizeof(sensor_uis));

    screen_home         = lv_obj_create(NULL);
    screen_settings     = lv_obj_create(NULL);
    screen_logs         = lv_obj_create(NULL);
    screen_about        = lv_obj_create(NULL);
    screen_screensaver  = lv_obj_create(NULL);

    lv_color_t bg = lv_color_hex(t->bg);

    lv_obj_set_style_bg_color(screen_home, bg, 0);
    lv_obj_set_style_bg_color(screen_settings, bg, 0);
    lv_obj_set_style_bg_color(screen_logs, bg, 0);
    lv_obj_set_style_bg_color(screen_about, bg, 0);

    lv_obj_set_style_bg_color(screen_screensaver, lv_color_hex(0x282A36), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_screensaver, LV_OPA_COVER, LV_PART_MAIN);

    ss_bouncer = lv_image_create(screen_screensaver);
    lv_image_set_src(ss_bouncer, &chas_logo_small);
    lv_obj_set_pos(ss_bouncer, LCD_H_RES / 3, LCD_V_RES / 3);

    lv_obj_t *home_root     = make_root(screen_home);
    lv_obj_t *settings_root = make_root(screen_settings);
    lv_obj_t *logs_root     = make_root(screen_logs);
    lv_obj_t *about_root    = make_root(screen_about);

    create_sidebar(home_root);
    create_sidebar(settings_root);
    create_sidebar(logs_root);
    create_sidebar(about_root);

    build_home_content(home_root);
    build_settings_content(settings_root);
    build_logs_content(logs_root);
    build_about_content(about_root);

    lv_screen_load(screen_home);
    lv_obj_update_layout(screen_home);
}

/* =======================
   UI EXTERNAL INTERFACE:
==========================*/

static void gui_update_network_status_async(void *arg)
{
    bool connected = Internet_Is_Connected();

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

    for (int i = 0; i < wifi_label_count; i++)
    {
        if (wifi_status_labels[i] == NULL)
            continue;

        lv_label_set_text(wifi_status_labels[i], text);
        lv_obj_set_style_text_color(wifi_status_labels[i], lv_color_hex(color), 0);
    }
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

        bool has_data = ui->sensor->data != NULL;
        bool open = false;

        if (has_data)
            open = *(bool *)ui->sensor->data;

        uint32_t status_color = !has_data ? 0x888888 : (open ? 0xFF5555 : 0x50FA7B);

        lv_label_set_text(ui->status_label, !has_data ? "UNKNOWN" : (open ? "OPEN" : "CLOSED"));
        lv_obj_set_style_text_color(ui->status_label, lv_color_hex(status_color), 0);
        lv_obj_set_style_bg_color(ui->dot, lv_color_hex(status_color), 0);
    }
}

/* =======================
        MAIN TASK:
==========================*/

void lvgl_task(void *arg)
{
    backlight_init();
    display_init();
    lvgl_port_init();

    esp_lcd_panel_io_handle_t touch_io = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &touch_io));

    esp_lcd_touch_config_t touch_config = {
        .x_max          = LCD_H_RES,
        .y_max          = LCD_V_RES,
        .rst_gpio_num   = GPIO_NUM_42,
        .int_gpio_num   = GPIO_NUM_4,
        .levels         = {
            .reset = 0,
            .interrupt = 0
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0
        },
    };

    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(touch_io, &touch_config, &touch_handle));

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    const esp_timer_create_args_t tick_timer_args = {
        .callback = lv_tick_cb,
        .name     = "lvgl_tick",
    };

    esp_timer_handle_t tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&tick_timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000));

    create_security_ui();

    lv_timer_create(update_sensor_ui_timer_cb, 250, NULL);

    last_input_time = lv_tick_get();

    xTaskCreate(
        GUI_Update_Network_Status,
        "GUIUpdateNetworkStatus",
        1024,
        NULL,
        8,
        NULL
    );

    while (1)
    {
        uint32_t now = lv_tick_get();

        if (screensaver_state == STATE_ACTIVE && (now - last_input_time) >= SCREENSAVER_TIMEOUT_MS)
        {
            screensaver_state = STATE_SCREENSAVER;
            lv_screen_load(screen_screensaver);
            lv_obj_invalidate(screen_screensaver);

            if (screensaver_timer == NULL)
            {
                screensaver_timer = lv_timer_create(screensaver_timer_cb, 16, NULL);
            }
            else 
            {
                lv_timer_resume(screensaver_timer);
            }


        }

        uint32_t delay_ms = lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(delay_ms > 0 ? delay_ms : 1));
    }
}