#ifndef GUI_TABS_H
#define GUI_TABS_H

#include "lvgl.h"

/** Home tab index. */
#define TAB_HOME                0

/** Environment tab index. */
#define TAB_ENV                 1

/** Settings tab index. */
#define TAB_SETTINGS            2

/** About tab index. */
#define TAB_ABOUT               3

/**
 * @brief Builds the Home tab.
 *
 * Creates the magnetic sensor overview and groups
 * sensors according to their configured placement.
 *
 * @param parent Parent LVGL container.
 */
void ui_build_tab_home(lv_obj_t *parent);

/**
 * @brief Builds the Environment tab.
 * 
 * Creates gauge-based displays for temperature, 
 * pressure, and humidity measurements.
 * 
 * @param parent Parent LVGL container.
 */
void ui_build_tab_env(lv_obj_t *parent);

/**
 * @brief Builds the Settings tab.
 * 
 * Creates the settings interface including theme selection, 
 * brightness control, and screensaver options.
 * 
 * @param parent Parent LVGL container.
 */
void ui_build_tab_settings(lv_obj_t *parent);

/**
 * @brief Builds the About tab.
 * 
 * Displays project information, contributor names
 * and repository details.
 * 
 * @param parent Parent LVGL container.
 */
void ui_build_tab_about(lv_obj_t *parent);


#endif