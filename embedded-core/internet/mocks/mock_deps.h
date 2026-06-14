#ifndef MOCK_DEPS_H
#define MOCK_DEPS_H
#define ESP_LOGI(tag, fmt, ...)

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <stddef.h>

/* Mock return values */

extern esp_err_t mock_nvs_flash_init_result;
extern esp_err_t mock_nvs_flash_erase_result;

extern esp_err_t mock_esp_netif_init_result;

extern esp_err_t mock_event_loop_create_default_result;
extern esp_err_t mock_event_handler_register_result;

extern esp_err_t mock_esp_wifi_init_result;
extern esp_err_t mock_esp_wifi_set_mode_result;
extern esp_err_t mock_esp_wifi_set_config_result;
extern esp_err_t mock_esp_wifi_start_result;
extern esp_err_t mock_esp_wifi_connect_result;
extern esp_err_t mock_esp_wifi_scan_start_result;

/* Mock arguments */

extern int mock_wifi_set_mode_argument;
extern wifi_config_t mock_wifi_config_argument;
extern esp_event_base_t mock_event_base_arguments[10];
extern int32_t mock_event_id_arguments[10];

/* Mock call counters */

extern int mock_esp_wifi_start_call_count;
extern int mock_nvs_flash_init_call_count;
extern int mock_nvs_flash_erase_call_count;
extern int mock_event_handler_register_call_count;

const char *esp_err_to_name(esp_err_t err);

size_t strlcpy(char *dst,
               const char *src,
               size_t size);

/* Logging */

#define ESP_LOGI(tag, fmt, ...)

#endif