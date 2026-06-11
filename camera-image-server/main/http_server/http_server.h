#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "esp_system.h"
#include "esp_http_server.h"

typedef struct
{
    char object_detection_server_url[256];
    char homehub_server_url[256];
} AddressConfig_t;

/// @brief Setup the HTTP server with the given address configuration
/// @param config Pointer to the address configuration structure
/// @return 0 if the server was successfully set up, otherwise an error code
int http_server_setup(AddressConfig_t *config);

#endif