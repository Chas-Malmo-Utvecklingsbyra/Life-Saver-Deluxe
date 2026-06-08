#include "packet.h"

#include <string.h>
#include <esp_log.h>
#include "allocator/arena.h"

#include "../../json/cJSON.h"

static const char* TAG = "Packet";

#define Packet_Job_Find_Job(str_value1, str_value2, return_value) \
    do { \
        if (strcmp((str_value1), (str_value2)) == 0) { \
            return (return_value); \
        } \
    } while(0) \


char* Packet_Job_To_String(Packet_Job job)
{
    switch (job)
    {
        case Packet_Job_Initialize:
        {
            return "Initialize"; 
        }
        case Packet_Job_Data:
        {
            return "Data"; 
        }
        case Packet_Job_Heartbeat:
        {
            return "Heartbeat"; 
        }
        case Packet_Job_Debug:
        {
            return "Debug";
        }
        default:
        {
            return NULL;
        }
    }
}

Packet_Job Packet_Job_From_String(const char* string)
{
    ESP_LOGI(TAG, "MADE IT IN HERE!");
    
    // Returns if job is found
    Packet_Job_Find_Job(string, "Initialize", Packet_Job_Initialize);
    Packet_Job_Find_Job(string, "Data", Packet_Job_Data);
    Packet_Job_Find_Job(string, "Heartbeat", Packet_Job_Heartbeat);

    return Packet_Job_Unknown;
}

char* Packet_Build(Packet_Job job, const char* message)
{
    cJSON* root = cJSON_CreateObject();

    if (root == NULL)
    {
        ESP_LOGE(TAG, "Root == NULL");
        Arena_Reset();
        return NULL;
    }

    char* packet_job_str = Packet_Job_To_String(job);
    if (packet_job_str == NULL)
    {
        Arena_Reset();
        ESP_LOGE(TAG, "Packet_Job_Str == NULL, check if you have added new enumerators to the switch case in Packet_Job_To_String");
        return NULL;
    }

    cJSON* str_result = cJSON_AddStringToObject(root, "job", packet_job_str);
    if (str_result == NULL)
    {
        ESP_LOGE(TAG, "Could not add job to JSON");
        Arena_Reset();
        return NULL;
    }
    
    cJSON *data_add = NULL;

    if (message == NULL)
    {
        data_add = cJSON_AddStringToObject(root, "data", "");
    }
    else
    {
        data_add = cJSON_AddStringToObject(root, "data", message);
    }

    if (data_add == NULL)
    {
        Arena_Reset();
        ESP_LOGE(TAG, "data_add == NULL");
        return NULL;
    }

    char* new_str = cJSON_Print(root);
    if (new_str == NULL)
    {
        ESP_LOGE(TAG, "new_str == NULL");
    }

    return new_str;
}