#ifndef GUI_TABS_H
#define GUI_TABS_H

#include "lvgl.h"

#define TAB_HOME                0
#define TAB_ENV                 1
#define TAB_SETTINGS            2
#define TAB_ABOUT               3

void ui_build_tab_home(lv_obj_t *parent);
void ui_build_tab_env(lv_obj_t *parent);
void ui_build_tab_settings(lv_obj_t *parent);
void ui_build_tab_about(lv_obj_t *parent);


#endif