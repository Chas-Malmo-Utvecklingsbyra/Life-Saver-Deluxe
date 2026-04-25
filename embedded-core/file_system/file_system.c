#include "file_system.h"

#include <esp_log.h>

#include "spiffs/spiffs.h"

static const char *TAG = "File System";

// This wont need to be mutexed as we only read data from it after Initialization.
static File_System singleton_file_system;


File_System_Error File_System_Initialize(File_System_Type type)
{
    static bool initialized = false;
    if (initialized)
    {
        return File_System_Error_Already_Initialized;
    }

    memset(&singleton_file_system, 0, sizeof(singleton_file_system));
    switch (type)
    {
        case File_System_Type_Spiffs:
        {
            if (!Spiffs_Initialize())
            {
                return File_System_Error_Spiffs;
            }
            singleton_file_system.type = File_System_Type_Spiffs;
            singleton_file_system.path = "/spiffs/";

            initialized = true;
            return File_System_Success;
        }
        default:
        {
            return File_System_Error_Unknown;
        }
    }

    initialized = true;
    return File_System_Success;
}


File_System_Error File_System_Write_File(const char* file_name, const char* text, const char* mode)
{
    char buffer[255] = {};
    snprintf(buffer, sizeof(buffer), "%s%s", singleton_file_system.path, file_name);
    FILE *file = NULL;

    if (singleton_file_system.type == File_System_Type_Spiffs)
    {
        file = fopen(buffer, mode);
        if (file == NULL)
        {
            return File_System_Error_File;
        }

        // if we only want to just create a file
        if (text == NULL)
        {
            fclose(file);
            return File_System_Success;
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

File_System_Error File_System_Read_File(const char* file_name, char* out_buffer, size_t length)
{
    char path[255] = {};
    snprintf(path, sizeof(path), "%s%s", singleton_file_system.path, file_name);
    
    FILE *file = NULL;


    if (singleton_file_system.type == File_System_Type_Spiffs)
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

bool File_System_File_Exists(const char* file_name)
{
    char path[255] = {};
    snprintf(path, sizeof(path), "%s%s", singleton_file_system.path, file_name);

    FILE *file = NULL;

    if (singleton_file_system.type == File_System_Type_Spiffs)
    {
        file = fopen(path, "r");
        if (file == NULL)
        {
            return false;
        }

        fclose(file);
        return true;
    }

    return false;
}

File_System_Type File_System_Get_Type()
{
    return singleton_file_system.type;
}