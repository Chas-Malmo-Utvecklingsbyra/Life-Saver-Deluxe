#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"



#include "internet/internet.h"
#include "tcp/tcp_server.h"

static const char* TAG = "Home-Hub";

void tcp_server_task(void* params)
{
    TCP_Server server = {};
    if (TCP_Server_Setup(&server, 6060) != TCP_Server_Success)
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

    while(1)
    {
        mdns_resp_init()
    }
}
