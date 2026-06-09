#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <esp_system.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

typedef enum
{
    HTTP_CONTENT_TYPE_OCTET_STREAM, // For binary data
    HTTP_CONTENT_TYPE_JPEG,
    HTTP_CONTENT_TYPE_PLAIN_TEXT,
    HTTP_CONTENT_TYPE_JSON,
} Http_Client_Content_Type_e;

/// @brief HTTP event handler
/// @param evt Pointer to the HTTP event structure
/// @return ESP_OK on success, ESP_FAIL on failure
esp_err_t http_event_handler(esp_http_client_event_t *evt);

/// @brief Perform an HTTP POST request
/// @param url The base URL of the server
/// @param path The path of the resource
/// @param content_type The HTTP content type enum
/// @param post_data The data to be posted
/// @param post_data_len The length of the data to be posted
/// @param response_buffer The buffer to store the response
/// @param buffer_size The size of the response buffer
/// @return ESP_OK on success, ESP_FAIL on failure
esp_err_t http_client_post(const char *url, const char *path, Http_Client_Content_Type_e content_type, const char *post_data, const uint16_t post_data_len, char *response_buffer, uint16_t buffer_size);

#endif