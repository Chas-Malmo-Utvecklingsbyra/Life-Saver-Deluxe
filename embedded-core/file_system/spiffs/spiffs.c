#include "spiffs.h"

#include <esp_spiffs.h>

bool Spiffs_Initialize()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 5, // 5 files open maximum
        .format_if_mount_failed = true
    };

    if (esp_vfs_spiffs_register(&conf) != ESP_OK)
    {
        return false;
    }

    return true;
}