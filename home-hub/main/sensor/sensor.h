#ifndef SENSOR_H
#define SENSOR_H

#define MAX_SENSORS 10

static Sensor sensors[MAX_SENSORS];

typedef enum
{
    Sensor_Type_Magnetic
} Sensor_Type;

typedef struct 
{
    Sensor_Type type;
    char* guid;
} Sensor;

#endif