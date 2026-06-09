#include "gui_rename.h"
#include "gui_themes.h"
#include "../sensor/sensor_settings.h"
#include "lvgl.h"

#include <string.h>
#include <stdio.h>

#define LCD_H_RES 1024
#define LCD_V_RES 600

extern uint8_t current_theme;
extern lv_display_t *lvgl_disp;

extern void ui_rebuild_all(void);

static lv_obj_t *rename_overlay = NULL;
static lv_obj_t *rename_ta      = NULL;
static lv_obj_t *rename_kb      = NULL;
static lv_obj_t *placement_dd   = NULL;
static SensorUi *rename_target  = NULL;

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
        lv_async_call((lv_async_cb_t)ui_rebuild_all, NULL);
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

void open_rename_overlay(SensorUi *ui)
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

void close_rename_overlay(void)
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