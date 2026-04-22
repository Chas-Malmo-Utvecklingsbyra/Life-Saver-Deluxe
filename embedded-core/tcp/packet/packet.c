#include "packet.h"

#include <string.h>

#include "../../json/cJSON.h"

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
        default:
        {
            return NULL;
        }
    }
}

Packet_Job Packet_Job_From_String(const char* string)
{
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
        return NULL;

    switch (job)
    {
        case Packet_Job_Initialize:
        {
            cJSON* str_result = cJSON_AddStringToObject(root, "job", "Initialize");
            if (str_result == NULL)
            {
                cJSON_Delete(root);
                return NULL;
            }

            break;
        }

        default:
        {
            return NULL;
            break;
        }
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
        cJSON_Delete(root);
        return NULL;
    }

    char* new_str = cJSON_Print(root);
    cJSON_Delete(root);

    return new_str;
}