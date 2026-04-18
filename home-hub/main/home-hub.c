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
        ESP_LOGI(TAG, "Failed to setup TCP_Server....");
    }
}

void app_main(void)
{
    Internet_Initialize("username", "password");
    xTaskCreate(tcp_server_task, "TCPServerTask", 4096, NULL, 10, NULL);
}
