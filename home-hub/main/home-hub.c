#include <stdio.h>

#include <mdns.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "internet/internet.h"
#include "tcp/tcp_server.h"
#include "json/cJSON.h"


static const char* TAG = "Home-Hub";

#define PORT 6060

void full_data_received(const char* data, uint16_t len)
{
    ESP_LOGI(TAG, "Received data: [%s]", data);

    cJSON* root = cJSON_Parse(data);
    if (root == NULL)
    {
        // not a valid json, we only accept JSON
        ESP_LOGI(TAG, "Data received was not a valid JSON. Returning...");
        return;
    }

    cJSON* job_json = cJSON_GetObjectItem(root, "job");
    if (job_json == NULL)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "Could not parse out JSON job... Returning...");
        return;
    }

    char* job_str = cJSON_GetStringValue(job_json);
    if (job_str == NULL)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "job_str is not a STRING.. Returning..");
        return;
    }
    
    ESP_LOGI(TAG, "Parsed out job: [%s]", job_str);


    cJSON_Delete(root);
}

void tcp_server_task(void* params)
{
    TCP_Server server = {};
    if (TCP_Server_Setup(&server, PORT, full_data_received) != TCP_Server_Success)
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
