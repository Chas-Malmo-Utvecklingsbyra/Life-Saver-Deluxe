#include "sensor.h"

#include <string.h>

#include <esp_log.h>

static const char* TAG = "Sensor";

static Sensor sensors[MAX_SENSORS];

void Sensor_Initialize_All()
{
    for (size_t i = 0; i < MAX_SENSORS; i++)
    {
        sensors[i].type = Sensor_Type_None;
        memset(sensors[i].guid, 0, sizeof(sensors[i].guid));
    }
}

bool Sensor_Spot_Empty(size_t index)
{
    if (sensors[index].type == Sensor_Type_None)
    {
        return true;
    }
    return false;
}

void Sensor_Add(Sensor_Type type, char* guid)
{
    for (size_t i = 0; i < MAX_SENSORS; i++)
    {
        if (Sensor_Spot_Empty(i))
        {
            sensors[i] = (Sensor){ .type = type };
            strncpy(sensors[i].guid, guid, 32);
            return;
        }
    }
    // Sensor spots are full
    ESP_LOGE(TAG, "Could not add a Sensor to a empty spot in Sensor_Add");
}