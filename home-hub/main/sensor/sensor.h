#ifndef SENSOR_H
#define SENSOR_H

#define MAX_SENSORS 10
#define MAX_SENSOR_CHAR_SIZE 128


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
    void* data;
} Sensor;

void Sensor_Initialize_All();
void Sensor_Add(Sensor_Type type, char* guid);
void Sensor_Print_All();
Sensor* Sensor_Get_By_UUID(const char* uuid);
Sensor* Sensor_Get_All();


#endif