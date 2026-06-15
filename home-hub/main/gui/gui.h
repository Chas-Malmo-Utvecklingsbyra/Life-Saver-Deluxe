#ifndef GUI_H
#define GUI_H

#include "internet/internet.h"

/**
 * @brief Rebuilds the complete user interface.
 *
 * Destroys existing screens and recreates all visual
 * components using the currently selected theme and
 * application state.
 */
void ui_rebuild_all(void);

/**
 * @brief Main LVGL task.
 *
 * Initializes the display, touch controller, LVGL,
 * screens, timers, and enters the main UI loop.
 *
 * @param arg FreeRTOS task argument.
 */
void lvgl_task(void *arg);

#endif