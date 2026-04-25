#include <stdio.h>

#include <mdns.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "internet/internet.h"
#include "tcp/tcp_server.h"
#include "json/cJSON.h"
#include "tcp/packet/packet.h"
#include "sensor/sensor.h"
#include "file_system/file_system.h"

#include "random/random.h"

static const char* TAG = "Home-Hub";

#define PORT 6060

void full_data_received(TCP_Server_Client* client)
{
    ESP_LOGI(TAG, "Received data: [%s]", client->data);

    cJSON* root = cJSON_Parse(client->data);
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

    cJSON* data_json = cJSON_GetObjectItem(root, "data");
    if (data_json == NULL)
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "Could not find 'data' in JSON.. Returning..");
        return;
    }
    
    ESP_LOGI(TAG, "Parsed out job: [%s]", job_str);

    switch (Packet_Job_From_String(job_str))
    {
        case Packet_Job_Initialize:
        {
            ESP_LOGI(TAG, "Made it in here!");

            char* data_str = cJSON_GetStringValue(data_json);
            if (data_str == NULL)
            {
                cJSON_Delete(root);
                ESP_LOGI(TAG, "data_str is not a STRING.. Returning..");
                return;
            }

            if (strcmp(data_str, "magnetic") == 0)
            {
                ESP_LOGI(TAG, "Added Sensor_Type_Magnetic to sensor array");
                Sensor_Add(Sensor_Type_Magnetic, "1234-5689-1023-4128"); // Guid generator should be added, or atleast some identifcation
            }

            char* packet = Packet_Build(Packet_Job_Debug, "Hello Magnetic-Sensor, you have been Initialized on Home Hub!");
            if (packet == NULL)
            {
                ESP_LOGE(TAG, "Packet == NULL in full_data_received, Packet_Job_Initialize");
                cJSON_Delete(root);
                return;
            }
            
            if (TCP_Server_Send(client, packet, strlen(packet)) != TCP_Server_Success)
            {
                ESP_LOGI(TAG, "Failed to Send data to Client");
            }

            break;
        }

        case Packet_Job_Heartbeat:
        {

            break;
        }

        case Packet_Job_Data:
        {

            break;
        }

        default:
        {
            break;
        }
    }


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
    File_System file_system = {};
    if (File_System_Initialize(&file_system, File_System_Type_Spiffs) != File_System_Success)
    {
        ESP_LOGE(TAG, "File system failed to Initialize! Returning from main.");
        return;
    }

    Sensor_Initialize_All();

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
