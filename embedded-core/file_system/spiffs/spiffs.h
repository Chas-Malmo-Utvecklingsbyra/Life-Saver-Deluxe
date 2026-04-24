#ifndef SPIFFS_H
#define SPIFFS_H

/**
 * @brief Initializes Spiffs
 *
 * This function initializes Spiffs, formats it too if it is not already formatted
 *
 * @return true if Successful otherwise returns false
 *
 * @warning Spiffs does not support subdirectories.
 * 
 */
bool Spiffs_Initialize();

#endif