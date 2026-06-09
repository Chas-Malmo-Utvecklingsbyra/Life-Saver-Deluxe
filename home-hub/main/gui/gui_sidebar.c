#include "gui_sidebar.h"
#include "gui_themes.h"
#include "gui_rename.h"
#include "lvgl.h"

#define LCD_V_RES 600

#define TAB_HOME                0
#define TAB_ENV                 1
#define TAB_SETTINGS            2
#define TAB_ABOUT               3

extern uint8_t current_theme;
extern lv_obj_t *main_tabview;
lv_obj_t *wifi_status_label = NULL;

static void on_nav_button_pressed(lv_event_t *e)
{
    close_rename_overlay();

    uint32_t tab_index = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    lv_tabview_set_active(main_tabview, tab_index, LV_ANIM_OFF);
}

void ui_build_sidebar(lv_obj_t *parent)
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

        lv_obj_add_event_cb(button, on_nav_button_pressed, LV_EVENT_PRESSED, (void*)(uintptr_t)tab_pages[i]);
    }
}