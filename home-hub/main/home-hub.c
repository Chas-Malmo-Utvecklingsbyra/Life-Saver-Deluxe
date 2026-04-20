#include <stdio.h>


#include <mdns.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "internet/internet.h"
#include "tcp/tcp_server.h"


static const char* TAG = "Home-Hub";

#define PORT 6060

void tcp_server_task(void* params)
{
    TCP_Server server = {};
    if (TCP_Server_Setup(&server, PORT) != TCP_Server_Success)
    {
        // Maybe we should do something here, trying to restart the Server maybe?
        ESP_LOGI(TAG, "Failed to setup TCP_Server....");
        vTaskDelete(NULL);
    }

    while (1)
    {
        TCP_Server_Work(&server);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    Internet_Initialize("username", "password");
    xTaskCreate(tcp_server_task, "TCPServerTask", 4096, NULL, 10, NULL);

    esp_err_t mdns_err = mdns_init();
    if (mdns_err != ESP_OK)
    {
        ESP_LOGE(TAG, "mdns_init failed, (code: %d)\n", mdns_err);
        return; 
    }
    mdns_hostname_set("homehub");
    mdns_service_add(NULL, "_http", "_tcp", PORT, NULL, 0);

    ESP_LOGI(TAG, "MSDN has successfully been initiated!");
}
