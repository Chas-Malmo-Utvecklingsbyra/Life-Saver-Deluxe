#include "gui_tabs.h"
#include "gui.h"
#include "gui_themes.h"
#include "gui_sensors.h"
#include "gui_backlight.h"
#include "gui_screensaver.h"
#include "../sensor/sensor.h"
#include "../sensor/sensor_settings.h"
#include "esp_log.h"

#include <stdio.h>
#include <inttypes.h>

#define TAG         "GUI"
#define LCD_H_RES   1204
#define LCD_V_RES   600


static void on_theme_button_pressed(lv_event_t *e)
{
    current_theme = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    ui_rebuild_all();
}

static void on_brightness_slider_changed(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    set_brightness((uint8_t)lv_slider_get_value(slider));
}

static void on_screensaver_toggle_changed(lv_event_t *e)
{
    lv_obj_t *toggle = lv_event_get_target(e);
    screensaver_enabled = lv_obj_has_state(toggle, LV_STATE_CHECKED);
}

static void on_screensaver_timeout_slider_changed(lv_event_t *e)
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

static lv_obj_t *ui_build_settings_card(lv_obj_t *parent, const char *title)
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

void ui_build_tab_home(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    sensor_ui_reset();

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

            ui_build_sensor_card(target_panel, sensor, buffer);
        }
    }

    ESP_LOGW(TAG, "Total magnetic sensor cards added: %zu", real_count);
}

void ui_build_tab_env(lv_obj_t *parent)
{
    const theme_t *t = &themes[current_theme];

    lv_obj_t *environment_section = lv_obj_create(parent);
    lv_obj_set_size(environment_section, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(environment_section, lv_color_hex(t->bg), 0);
    lv_obj_set_style_border_width(environment_section, 0, 0);
    lv_obj_set_flex_flow(environment_section, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(environment_section, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(environment_section, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(environment_section, LV_OBJ_FLAG_SCROLLABLE);

    ui_build_env_card(environment_section, "Temperature", -20, 60, &bme280_ui[0]);
    ui_build_env_card(environment_section, "Pressure", 900, 1500, &bme280_ui[1]);
    ui_build_env_card(environment_section, "Humidity", 0, 100, &bme280_ui[2]);
}

void ui_build_tab_settings(lv_obj_t *parent)
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
      
    lv_obj_t *theme_card = ui_build_settings_card(section, "Color Themes");
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

        lv_obj_add_event_cb(btn, on_theme_button_pressed, LV_EVENT_PRESSED, (void *)(uintptr_t)i);
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

    lv_obj_t *brightness_card = ui_build_settings_card(bottom_row, "Brightness");
    lv_obj_set_flex_grow(brightness_card, 1);
    lv_obj_set_height(brightness_card, LV_PCT(100));

    lv_obj_t *slider = lv_slider_create(brightness_card);
    lv_obj_set_width(slider, LV_PCT(90));
    lv_slider_set_range(slider, BACKLIGHT_MIN_PCT, 100);
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, on_brightness_slider_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *ss_card = ui_build_settings_card(bottom_row, "Screensaver settings");
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
    lv_obj_add_event_cb(sw, on_screensaver_toggle_changed, LV_EVENT_VALUE_CHANGED, NULL);

    char timeout_buf[32];
    snprintf(timeout_buf, sizeof(timeout_buf), "Timeout: %"PRIu32" seconds", screensaver_timeout_ms / 1000);

    lv_obj_t *timeout_label = lv_label_create(ss_card);
    lv_label_set_text(timeout_label, timeout_buf);
    lv_obj_set_style_text_color(timeout_label, lv_color_hex(t->text), 0);

    lv_obj_t *ss_slider = lv_slider_create(ss_card);
    lv_obj_set_width(ss_slider, LV_PCT(90));
    lv_slider_set_range(ss_slider, 10, 300);
    lv_slider_set_value(ss_slider, screensaver_timeout_ms / 1000, LV_ANIM_OFF);
    lv_obj_add_event_cb(ss_slider, on_screensaver_timeout_slider_changed, LV_EVENT_VALUE_CHANGED, timeout_label);
}

void ui_build_tab_about(lv_obj_t *parent)
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
        "Par Lundh\n\n"
        "For more info about our projects\n"
        "please visit our github:\n"
        "https://github.com/Chas-Malmo-Utvecklingsbyra\n"
    );

    lv_obj_set_style_text_color(body, lv_color_hex(t->text), 0);
    lv_obj_set_style_text_font(body, t->font_small, 0);
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(body, LV_PCT(100));
}