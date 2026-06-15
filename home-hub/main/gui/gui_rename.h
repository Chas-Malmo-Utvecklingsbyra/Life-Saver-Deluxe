#ifndef GUI_RENAME_H
#define GUI_RENAME_H

#include "lvgl.h"
#include "gui_sensors.h"

/**
 * @brief Opens the sensor rename dialog.
 *
 * Creates a modal overlay that allows the user to modify
 * the sensor display name and placement category.
 *
 * @param ui Pointer to the sensor UI entry being edited.
 */
void open_rename_overlay(SensorUi *ui);

/**
 * @brief Closes the rename dialog.
 *
 * Removes the rename overlay and clears all internal
 * references associated with the dialog.
 */
void close_rename_overlay(void);

#endif