#ifndef GUI_SIDEBAR_H
#define GUI_SIDEBAR_H

#include "lvgl.h"

/**
 * @brief Label displaying WiFi status in sidebar UI.
 *
 * Updated asynchronously based on connectivity state.
 */
extern lv_obj_t *wifi_status_label;

/**
 * @brief Builds the left navigation sidebar.
 *
 * Creates navigation buttons, title, and network status indicator.
 *
 * @param parent Parent LVGL container.
 */
void ui_build_sidebar(lv_obj_t *parent);

#endif