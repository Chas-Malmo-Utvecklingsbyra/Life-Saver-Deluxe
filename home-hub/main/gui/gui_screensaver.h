#ifndef GUI_SCREENSAVER_H
#define GUI_SCREENSAVER_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

/**
 * @brief Runtime state for the screensaver.
 */
typedef struct
{
    bool active;
    lv_timer_t *anim_timer;
} ScreensaverControl;

extern ScreensaverControl ss;
extern uint32_t screensaver_timeout_ms;
extern bool screensaver_enabled;

/**
 * @brief Builds the screensaver screen.
 *
 * Creates the graphical objects used by the screensaver.
 *
 * @param screen LVGL screen object that will contain
 * the screensaver content.
 */
void screensaver_build(lv_obj_t *screen);

/**
 * @brief Activates the screensaver.
 *
 * Switches to the screensaver screen, starts the
 * animation timer, and pauses periodic UI updates.
 *
 * @param screen_screensaver Screensaver screen.
 * @param screen_main Main application screen.
 * @param sensor_update_timer Timer used for UI updates.
 */
void screensaver_enter(lv_obj_t *screen_screensaver, lv_obj_t *screen_main, lv_timer_t *sensor_update_timer);

/**
 * @brief Deactivates the screensaver.
 *
 * Returns to the main UI screen and resumes paused
 * update timers.
 *
 * @param screen_main Main application screen.
 * @param ui_update_timer Timer used for UI updates.
 */
void screensaver_exit(lv_obj_t *screen_main, lv_timer_t *ui_update_timer);

#endif