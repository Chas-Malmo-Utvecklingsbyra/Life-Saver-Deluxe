#include "packet.h"

#include "../../json/cJSON.h"

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