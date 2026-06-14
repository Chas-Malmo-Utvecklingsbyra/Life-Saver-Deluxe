#include "mock_deps.h"
#include <string.h>

/* Mock return values */

esp_err_t mock_nvs_flash_init_result = ESP_OK;
esp_err_t mock_nvs_flash_erase_result = ESP_OK;

esp_err_t mock_esp_netif_init_result = ESP_OK;

esp_err_t mock_event_loop_create_default_result = ESP_OK;
esp_err_t mock_event_handler_register_result = ESP_OK;

esp_err_t mock_esp_wifi_init_result = ESP_OK;
esp_err_t mock_esp_wifi_set_mode_result = ESP_OK;
esp_err_t mock_esp_wifi_set_config_result = ESP_OK;
esp_err_t mock_esp_wifi_start_result = ESP_OK;
esp_err_t mock_esp_wifi_connect_result = ESP_OK;
esp_err_t mock_esp_wifi_scan_start_result = ESP_OK;

int mock_wifi_set_mode_argument = -1;
wifi_config_t mock_wifi_config_argument;

esp_event_base_t mock_event_base_arguments[10] = {0};
int32_t mock_event_id_arguments[10] = {0};

int mock_esp_wifi_start_call_count = 0;
int mock_nvs_flash_init_call_count = 0;
int mock_nvs_flash_erase_call_count = 0;
int mock_event_handler_register_call_count = 0;

/* Mock implementations */

esp_err_t nvs_flash_init(void)
{
    mock_nvs_flash_init_call_count++;

    return mock_nvs_flash_init_result;
}

esp_err_t nvs_flash_erase(void)
{
    mock_nvs_flash_erase_call_count++;

    return mock_nvs_flash_erase_result;
}

esp_err_t esp_netif_init(void)
{
    return mock_esp_netif_init_result;
}

void *esp_netif_create_default_wifi_sta(void)
{
    return NULL;
}

esp_err_t esp_event_loop_create_default(void)
{
    return mock_event_loop_create_default_result;
}

esp_err_t esp_event_handler_instance_register(
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_handler,
    void *event_handler_arg,
    void *instance)
{
    mock_event_base_arguments[mock_event_handler_register_call_count] =
        event_base;

    mock_event_id_arguments[mock_event_handler_register_call_count] =
        event_id;

    mock_event_handler_register_call_count++;

    (void)event_handler;
    (void)event_handler_arg;
    (void)instance;

    return mock_event_handler_register_result;
}

esp_err_t esp_wifi_init(const wifi_init_config_t *cfg)
{
    (void)cfg;
    return mock_esp_wifi_init_result;
}

esp_err_t esp_wifi_set_mode(int mode)
{
    mock_wifi_set_mode_argument = mode;

    return mock_esp_wifi_set_mode_result;
}

esp_err_t esp_wifi_set_config(int interface, const wifi_config_t *config)
{
    (void)interface;

    mock_wifi_config_argument = *config;

    return mock_esp_wifi_set_config_result;
}

esp_err_t esp_wifi_start(void)
{
    mock_esp_wifi_start_call_count++;

    return mock_esp_wifi_start_result;
}

esp_err_t esp_wifi_connect(void)
{
    return mock_esp_wifi_connect_result;
}

esp_err_t esp_wifi_scan_start(void *config, bool block)
{
    (void)config;
    (void)block;

    return mock_esp_wifi_scan_start_result;
}

const char *esp_err_to_name(esp_err_t err)
{
    (void)err;
    return "Mock Error";
}

size_t strlcpy(char *dst,
               const char *src,
               size_t size)
{
    size_t len = strlen(src);

    if (size > 0)
    {
        strncpy(dst, src, size - 1);
        dst[size - 1] = '\0';
    }

    return len;
}