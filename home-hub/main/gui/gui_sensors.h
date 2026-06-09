#ifndef GUI_SENSORS_H
#define GUI_SENSORS_H

#include <stddef.h>
#include <stdbool.h>
#include "lvgl.h"
#include "../sensor/sensor.h"
#include "bme280/bme280.h"

#define SENSOR_NAME_MAX 31

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

typedef struct
{
    lv_obj_t *card;
    lv_obj_t *name_label;
    lv_obj_t *data_label;
    lv_obj_t *arc;
    char data[20];
} BME280Ui;

extern SensorUi sensor_uis[];
extern size_t sensor_ui_count;
extern BME280Ui bme280_ui[3];


lv_obj_t *ui_build_sensor_card(lv_obj_t *parent, Sensor *sensor, const char *name);
lv_obj_t *ui_build_env_card(lv_obj_t *parent, const char *title, int32_t range_min, int32_t range_max, BME280Ui *out);

void on_ui_poll_timer(lv_timer_t *timer);
void sensor_ui_reset(void);

#endif