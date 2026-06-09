#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include "esp_system.h"
#include "esp_http_server.h"

typedef struct
{
    char object_detection_server_url[256];
    char homehub_server_url[256];
} CameraConfig_t;

int http_server_setup(CameraConfig_t *config);

#endif