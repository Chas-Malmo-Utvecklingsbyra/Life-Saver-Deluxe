#ifndef INTERNET_H
#define INTERNET_H

/**
 * 
 * @brief Initializes WIFI
 * 
 * @param ssid WIFI username
 * @param password WIFI password
 * 
 * @note
 * 
 * Use Internet_Is_Connected(), to check if wifi is available
 * 
 *
 */
void Internet_Initialize(const char *ssid, const char *password);

/**
 * 
 * @brief Checks if ESP32 is connected to Internet.
 * 
 *
 * @return
 * Returns true if connected to Internet, otherwise returns false.
 *  
 * @note
 * This method is thread safe.
 * 
 * 
 *
 */
bool Internet_Is_Connected();

#endif