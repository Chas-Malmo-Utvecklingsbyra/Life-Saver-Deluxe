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
#include "gui/gui.h"

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

            char buffer[RANDOM_MAX_UUID_V4_LENGTH];
            Random_Generate_UUID_v4(buffer);

            char* packet = Packet_Build(Packet_Job_Initialize, buffer);
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
            free(packet);

            if (!File_System_File_Exists("sensors"))
            {
                if (File_System_Write_File("sensors", buffer, "w") != File_System_Success)
                {
                    ESP_LOGE(TAG, "Failed to write to sensors file");
                    break;
                }

                ESP_LOGI(TAG, "Sucessfully wrote to sensors file");
            }
            else
            {
                if (File_System_Write_File("sensors", buffer, "a") != File_System_Success)
                {
                    ESP_LOGE(TAG, "Failed to append to sensors file");
                    break;
                }

                ESP_LOGI(TAG, "Sucessfully appended to sensors file");
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
    if (File_System_Initialize(File_System_Type_Spiffs) != File_System_Success)
    {
        ESP_LOGE(TAG, "File system failed to Initialize! Returning from main.");
        return;
    }

    //remove("/spiffs/sensors");
    xTaskCreatePinnedToCore(lvgl_task, "lvgl", 32768, NULL, 5, NULL, 0);

    Sensor_Initialize_All();

    if (File_System_File_Exists("sensors"))
    {
        if (File_System_Get_Type() != File_System_Type_Spiffs)
        {
            ESP_LOGE(TAG, "This code needs to implement other filesystem, right now it only supports SPIFFS");
            return;
        }

        FILE *f = fopen("/spiffs/sensors", "r");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open /spiffs/sensors");
            return;
        }

        char line[RANDOM_MAX_UUID_V4_LENGTH];
        while (fgets(line, sizeof(line), f) != NULL) {
            ESP_LOGE(TAG, "Sensor UUID4: %s", line);
            Sensor_Add(Sensor_Type_Unassigned, line);
        }
        fclose(f);
    }
    else
    {
        ESP_LOGI(TAG, "Sensors file does not exist!");
    }

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
