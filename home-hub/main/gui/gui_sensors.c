#include "gui_sensors.h"
#include "gui_rename.h"
#include "gui_themes.h"
#include "gui.h"
#include "../sensor/sensor_settings.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <string.h>
#include <inttypes.h>

#define TAG "GUI_SENSORS"

extern uint8_t current_theme;
extern lv_obj_t *main_tabview;

extern bme280_meas_t meas;
extern bool bme280_running;
extern SemaphoreHandle_t env_sensor_mutex;
extern bool ui_rebuilding;

SensorUi sensor_uis[MAX_SENSORS];
size_t sensor_ui_count = 0;
BME280Ui bme280_ui[3];

static uint32_t wifi_counter = 0;

void sensor_ui_reset(void)
{
    sensor_ui_count = 0;
    memset(sensor_uis, 0, sizeof(sensor_uis));
}

/**
 * @brief Async handler that opens rename dialog.
 *
 * Executed on LVGL async context to ensure thread safety.
 *
 * @param arg Pointer to SensorUi instance.
 */
static void on_sensor_tap_async(void *arg)
{
    SensorUi *ui = (SensorUi *)arg;

    if (ui == NULL) return;
    if (ui_rebuilding) return;
    if (ui->card == NULL) return;

    open_rename_overlay(ui);
}

/**
 * @brief LVGL event callback for sensor card taps.
 *
 * Validates active tab context and schedules async rename overlay.
 *
 * @param e LVGL event object.
 */
static void on_sensor_card_tapped(lv_event_t *e)
{
    SensorUi *ui = (SensorUi *)lv_event_get_user_data(e);
    if (ui == NULL) return;

    if (lv_tabview_get_tab_active(main_tabview) != 0) return;

    lv_async_call(on_sensor_tap_async, ui);
}

lv_obj_t *ui_build_sensor_card(lv_obj_t *parent, Sensor *sensor, const char *name)
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

    lv_obj_add_event_cb(card, on_sensor_card_tapped, LV_EVENT_CLICKED, &sensor_uis[sensor_ui_count]);

    sensor_ui_count++;

    return card;
}

lv_obj_t *ui_build_env_card(lv_obj_t *parent, const char *title, int32_t range_min, int32_t range_max, BME280Ui *out)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, LV_PCT(30), LV_PCT(80));
    lv_obj_set_style_bg_color(card, lv_color_hex(t->sensor_bg), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_set_style_pad_gap(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *name = lv_label_create(card);
    lv_label_set_text(name, title);
    lv_obj_set_style_text_color(name, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(name, t->font_normal, 0);

    lv_obj_t *arc_cont = lv_obj_create(card);
    lv_obj_set_size(arc_cont, 180, 180);
    lv_obj_set_style_bg_opa(arc_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(arc_cont, 0, 0);
    lv_obj_set_style_pad_all(arc_cont, 0, 0);
    lv_obj_clear_flag(arc_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *arc = lv_arc_create(arc_cont);
    lv_obj_set_size(arc, 160, 160);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_range(arc, range_min, range_max);
    lv_arc_set_value(arc, range_min);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_color(arc, lv_color_hex(t->button), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(t->button_pressed), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 12, LV_PART_INDICATOR);

    lv_obj_t *val_label = lv_label_create(arc_cont);
    lv_label_set_text(val_label, "--");
    lv_obj_set_style_text_color(val_label, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(val_label, t->font_large, 0);
    lv_obj_center(val_label);

    out->card       = card;
    out->name_label = name;
    out->data_label = val_label;
    out->arc        = arc;

    return card;
}

void on_ui_poll_timer(lv_timer_t *timer)
{
    if (ui_rebuilding) return;

    // Magnetic Sensors
    for (size_t i = 0; i < sensor_ui_count; i++)
    {
        SensorUi *ui = &sensor_uis[i];

        if (ui->sensor == NULL)
            continue;

        void *data_ptr = ui->sensor->data;
        bool has_data = (data_ptr != NULL);
        bool open = has_data && *(bool *)data_ptr;

        bool changed = (!ui->initialized || ui->last_has_data != has_data || ui->last_open != open);
        if (!changed)
            continue;

        ui->initialized = true;
        ui->last_has_data = has_data;
        ui->last_open = open;

        uint32_t status_color = !has_data ? 0x888888 : (open ? 0xFF5555 : 0x50FA7B);

        if (ui->status_label == NULL || ui->dot == NULL || ui->card == NULL) continue;
        lv_label_set_text(ui->status_label, !has_data ? "UNKNOWN" : (open ? "OPEN" : "CLOSED"));
        lv_obj_set_style_text_color(ui->status_label, lv_color_hex(status_color), 0);
        lv_obj_set_style_bg_color(ui->dot, lv_color_hex(status_color), 0);
    }

    // BME280 Sensor
    if (bme280_ui[0].data_label == NULL) return;

    if (bme280_running == false)
    {
        lv_label_set_text(bme280_ui[0].data_label, "--");
        lv_label_set_text(bme280_ui[1].data_label, "--");
        lv_label_set_text(bme280_ui[2].data_label, "--");
        return;
    }

    int32_t temp_raw;
    int32_t press_hpa;
    int32_t hum_pct;
    if (xSemaphoreTake(env_sensor_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        temp_raw = meas.T;
        press_hpa = meas.P / 256 / 100;
        hum_pct = meas.H / 1024;

        xSemaphoreGive(env_sensor_mutex);
    }
    else
    {
        ESP_LOGW(TAG, "Mutex timeout in on_ui_poll_timer");
        return;
    }

    snprintf(bme280_ui[0].data, sizeof(bme280_ui[0].data), "%ld.%01ld°C", temp_raw / 100, labs(temp_raw % 100) / 10);
    snprintf(bme280_ui[1].data, sizeof(bme280_ui[1].data), "%ldhPa", press_hpa);
    snprintf(bme280_ui[2].data, sizeof(bme280_ui[2].data), "%ld%%", hum_pct);

    lv_label_set_text(bme280_ui[0].data_label, bme280_ui[0].data);
    lv_label_set_text(bme280_ui[1].data_label, bme280_ui[1].data);
    lv_label_set_text(bme280_ui[2].data_label, bme280_ui[2].data);

    int32_t temp_c = temp_raw / 100;
    lv_arc_set_value(bme280_ui[0].arc, temp_c);
    lv_arc_set_value(bme280_ui[1].arc, press_hpa);
    lv_arc_set_value(bme280_ui[2].arc, hum_pct);

    wifi_counter++;

    if (wifi_counter >= 2)
    {
        wifi_counter = 0;
        on_wifi_status_update();
    }
}

