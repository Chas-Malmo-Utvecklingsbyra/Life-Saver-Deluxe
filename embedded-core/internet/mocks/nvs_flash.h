#ifndef MOCK_NVS_FLASH_H
#define MOCK_NVS_FLASH_H

#include "esp_wifi.h"

#define ESP_ERR_NVS_NO_FREE_PAGES 1001
#define ESP_ERR_NVS_NEW_VERSION_FOUND 1002

/* Function prototypes */

esp_err_t nvs_flash_init(void);
esp_err_t nvs_flash_erase(void);

#endif