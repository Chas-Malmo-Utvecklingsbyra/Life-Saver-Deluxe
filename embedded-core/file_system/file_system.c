#include "file_system.h"

#include <esp_log.h>

#include "spiffs/spiffs.h"

static const char *TAG = "File System";

File_System_Error File_System_Initialize(File_System *file_system, File_System_Type type)
{
    if (file_system == NULL)
    {
        ESP_LOGE(TAG, "file_system parameter is NULL");
        return false;
    }

    switch (type)
    {
        case File_System_Type_Spiffs:
        {
            if (!Spiffs_Initialize())
            {
                return File_System_Error_Spiffs;
            }
            file_system->type = File_System_Type_Spiffs;
            file_system->path = "/spiffs/";
            return File_System_Success;
        }
        default:
        {
            return File_System_Error_Unknown;
        }
    }


    return File_System_Success;
}


File_System_Error File_System_Write_File(File_System *file_system, const char* file_name, const char* text)
{
    char buffer[255] = {};
    snprintf(buffer, sizeof(buffer), "%s%s", file_system->path, file_name);
    
    FILE *file = NULL;

    if (file_system->type == File_System_Type_Spiffs)
    {
        file = fopen(buffer, "w");
        if (file == NULL)
        {
            return File_System_Error_File;
        }

        if (fprintf(file, text) < 0)
        {
            fclose(file);
            file = NULL;

            return File_System_Error_File_Write;
        }

    }

    if (file != NULL)
    {
        fclose(file);
    }

    return File_System_Success;
}

File_System_Error File_System_Read_File(File_System *file_system, const char* file_name, char* out_buffer, size_t length)
{
    char path[255] = {};
    snprintf(path, sizeof(path), "%s%s", file_system->path, file_name);
    
    FILE *file = NULL;


    if (file_system->type == File_System_Type_Spiffs)
    {
        file = fopen(path, "r");
        if (file == NULL)
        {
            return File_System_Error_File;
        }

        if (fgets(out_buffer, length, file) == NULL)
        {
            fclose(file);
            return File_System_Error_File_Read;
        }
    }

    fclose(file);

    return File_System_Success;
}