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

typedef enum
{
    PLACEMENT_DOOR,
    PLACEMENT_WINDOW,
    PLACEMENT_UNASSIGNED
} SensorPlacement;

typedef struct 
{
    Sensor_Type type;
    char guid[RANDOM_MAX_UUID_V4_LENGTH];
    void* data;
    SensorPlacement placement;
} Sensor;

/**
 * @brief Initializes the Sensor array
 */
void Sensor_Initialize_All();

/**
 * @brief Adds a Sensor to the array.
 *
 * Adds a Sensor to the array, Array size should be less than MAX_SENSORS otherwise it will fail to add.
 *
 * @param type The type of the Sensor.
 * @param guid The UUID of the Sensor to add.

 * @note guid means exact same as UUID in this case
 *
 */
void Sensor_Add(Sensor_Type type, char* guid);

/**
 * @brief Prints info about ALL the sensors
 */
void Sensor_Print_All();

/**
 * @brief Get a Sensor by UUID
 *
 * Loops through the Sensor array and checks if UUID is equal and then returns 
 * the pointer to it.
 * 
 * @param uuid The UUID that needs to match the sensor that is looked for.
 * @return Pointer to the sensor in the array.
 */
Sensor* Sensor_Get_By_UUID(const char* uuid);

/**
 * @brief Get the Sensor Array
 *
 * @return Pointer to the sensor array.
 *
 * @note This function returns a pointer of a singleton. 
 *
 */
Sensor* Sensor_Get_All();


#endif