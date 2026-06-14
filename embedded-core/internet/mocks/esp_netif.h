#ifndef MOCK_ESP_NETIF_H
#define MOCK_ESP_NETIF_H

#include "esp_wifi.h"

/* Function prototypes */

esp_err_t esp_netif_init(void);

void *esp_netif_create_default_wifi_sta(void);

#endif