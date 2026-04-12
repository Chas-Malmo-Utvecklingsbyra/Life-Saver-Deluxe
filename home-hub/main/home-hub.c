#include <stdio.h>

#include <nvs_flash.h>
#include <esp_now.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "internet/internet.h"

static const char *TAG = "Home-Hub";

void recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    ESP_LOGI(TAG, "Received: %d bytes", len);
    ESP_LOGI(TAG, "Data: %s", data);
}

void app_main(void)
{
    //Internet_Initialize("network name", "password");
    nvs_flash_init();
    Internet_Initialize_Minimal();

    if (esp_now_init() != ESP_OK)
    {
        ESP_LOGI(TAG, "Initialization of ESP-NOW failed!");
        return;
    }

    esp_now_register_recv_cb(recv_cb);


    uint8_t mac[6];
	esp_wifi_get_mac(WIFI_IF_STA, mac);

    ESP_LOGI("MAC", "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);


    uint8_t primary;
	wifi_second_chan_t second;

	esp_wifi_get_channel(&primary, &second);

	ESP_LOGI("WIFI", "Current channel: %d", primary);


    // Just chill since we have nothing else to do!
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
