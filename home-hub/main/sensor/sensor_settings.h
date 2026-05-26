#ifndef SENSOR_SETTINGS_H
#define SENSOR_SETTINGS_H

#include <stddef.h>

void sensor_names_init(void);

bool sensor_names_get(const char *guid, char *out, size_t out_size);

void sensor_names_set(const char *guid, const char *name);

void sensor_placement_init(void);

bool sensor_placement_set(const char *guid, SensorPlacement placement);

bool sensor_placement_get(const char *guid, SensorPlacement *placement);

#endif