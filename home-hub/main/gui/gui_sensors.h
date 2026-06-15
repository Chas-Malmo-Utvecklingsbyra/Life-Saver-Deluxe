#ifndef GUI_SENSORS_H
#define GUI_SENSORS_H

#include <stddef.h>
#include <stdbool.h>
#include "lvgl.h"
#include "../sensor/sensor.h"
#include "bme280/bme280.h"

/**
 * @brief Maximum length of a sensor display name (excluding null terminator).
 */
#define SENSOR_NAME_MAX 31

/**
 * @brief UI representation of a magnetic sensor.
 *
 * Holds LVGL objects bound to a sensor instance, along with cached state
 * used to minimize UI updates.
 */
typedef struct
{
    lv_obj_t *card;
    lv_obj_t *dot;
    lv_obj_t *name_label;
    lv_obj_t *status_label;
    Sensor   *sensor;
    char display_name[SENSOR_NAME_MAX + 1];
    bool last_open;
    bool last_has_data;
    bool initialized;
} SensorUi;

/**
 * @brief UI representation of BME280 environmental sensor data.
 *
 * Stores LVGL objects and cached formatted data strings.
 */
typedef struct
{
    lv_obj_t *card;
    lv_obj_t *name_label;
    lv_obj_t *data_label;
    lv_obj_t *arc;
    char data[20];
} BME280Ui;

/**
 * @brief Global UI sensor array.
 *
 * Contains all active sensor UI elements currently rendered.
 */
extern SensorUi sensor_uis[];

/**
 * @brief Number of active sensor UI elements.
 */
extern size_t sensor_ui_count;

/**
 * @brief UI containers for BME280 sensor channels (temperature, pressure, humidity).
 */
extern BME280Ui bme280_ui[3];

/**
 * @brief Builds a UI card for a magnetic sensor.
 *
 * Creates a clickable LVGL card bound to a Sensor instance.
 *
 * @param parent Parent LVGL container.
 * @param sensor Sensor backend object.
 * @param name Fallback display name if no stored name exists.
 *
 * @return Pointer to created LVGL object, or NULL on failure.
 */
lv_obj_t *ui_build_sensor_card(lv_obj_t *parent, Sensor *sensor, const char *name);

/**
 * @brief Builds a UI card for environmental sensor data.
 *
 * Creates a circular gauge-based display for BME280 values.
 *
 * @param parent Parent LVGL container.
 * @param title Display title of the metric.
 * @param range_min Minimum value of gauge.
 * @param range_max Maximum value of gauge.
 * @param out Output struct storing references to created UI objects.
 *
 * @return Pointer to created LVGL object.
 */
lv_obj_t *ui_build_env_card(lv_obj_t *parent, const char *title, int32_t range_min, int32_t range_max, BME280Ui *out);

/**
 * @brief Periodic UI update handler.
 *
 * Polls sensor backend state and updates LVGL objects accordingly.
 * Handles both magnetic sensors and BME280 environmental readings.
 *
 * @param timer LVGL timer triggering this callback.
 */
void on_ui_poll_timer(lv_timer_t *timer);

/**
 * @brief Resets all sensor UI state.
 *
 * Clears internal arrays and resets counters before rebuilding UI.
 *
 * @note Does not delete LVGL objects; only resets tracking state.
 */
void sensor_ui_reset(void);

#endif