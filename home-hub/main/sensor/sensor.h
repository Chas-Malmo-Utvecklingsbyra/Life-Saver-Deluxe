#ifndef SENSOR_H
#define SENSOR_H

#define MAX_SENSORS 10

#include "random/random.h"

typedef enum
{
    Sensor_Type_Magnetic,
    Sensor_Type_Unassigned,
    Sensor_Type_None // Should not be used
} Sensor_Type;

typedef struct 
{
    Sensor_Type type;
    char guid[RANDOM_MAX_UUID_V4_LENGTH];
} Sensor;

void Sensor_Initialize_All();
void Sensor_Add(Sensor_Type type, char* guid);


#endif