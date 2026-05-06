#ifndef INTERNET_H
#define INTERNET_H

#include <stdbool.h>

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

// OFFLINE MODE! 

typedef enum
{
    NETWORK_CONNECTING,
    NETWORK_ONLINE,
    NETWORK_OFFLINE

   
}  network_state_t;

// Global network state
extern network_state_t g_network_state;


network_state_t Internet_Get_State(void);

#endif