#ifndef MOCK_ESP_WIFI_H
#define MOCK_ESP_WIFI_H

#include <stdbool.h>
#include <stdint.h>

/* esp_err_t stub */
typedef int esp_err_t;

#define ESP_OK    0
#define ESP_FAIL -1

#define ESP_ERROR_CHECK(x) (x)

/* WiFi mode */
#define WIFI_MODE_STA 1

/* WiFi interface */
#define WIFI_IF_STA 0

/* Event base */
#define WIFI_EVENT 1

/* WiFi events */
#define WIFI_EVENT_STA_START        100
#define WIFI_EVENT_STA_DISCONNECTED 101

typedef struct
{
    int reason;
} wifi_event_sta_disconnected_t;

typedef struct
{
    char ssid[32];
    char password[64];
} wifi_sta_config_t;

typedef struct
{
    wifi_sta_config_t sta;
} wifi_config_t;

typedef struct
{
    int dummy;
} wifi_init_config_t;

#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){0})

/* Function prototypes */

esp_err_t esp_wifi_init(const wifi_init_config_t *cfg);
esp_err_t esp_wifi_set_mode(int mode);
esp_err_t esp_wifi_set_config(int interface, const wifi_config_t *config);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_connect(void);
esp_err_t esp_wifi_scan_start(void *config, bool block);

#endif