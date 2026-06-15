#include "commands.h"

#include <stdint.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "Commands";

int Command_Handle_Fragment(int argc, char **argv)
{
    uint32_t caps = MALLOC_CAP_SPIRAM;

    size_t free_heap = heap_caps_get_free_size(caps);
    size_t largest_block = heap_caps_get_largest_free_block(caps);
    size_t min_free = heap_caps_get_minimum_free_size(caps);

    float fragmentation = 0.0f;
    if (free_heap > 0) 
    {
        fragmentation = 100.0f - ((float)largest_block / (float)free_heap * 100.0f);
    }

    ESP_LOGI(TAG, "===== Heap Fragmentation =====");
    ESP_LOGI(TAG, "Free heap:          %u bytes", (unsigned int)free_heap);
    ESP_LOGI(TAG, "Largest free block: %u bytes", (unsigned int)largest_block);
    ESP_LOGI(TAG, "Minimum free heap:  %u bytes", (unsigned int)min_free);
    ESP_LOGI(TAG, "Fragmentation:      %.1f%%", fragmentation);

    return 0;
}

int Command_Handle_Uptime(int argc, char **argv)
{
    ESP_LOGI(TAG, "===== Uptime =====");
    int64_t uptime_us = esp_timer_get_time();

    uint64_t total_seconds = uptime_us / 1000000ULL;

    uint32_t hours   = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;

    ESP_LOGI(TAG, "%02lu:%02lu:%02lu",
        (unsigned long)hours,
        (unsigned long)minutes,
        (unsigned long)seconds);
    return 0;
}

int Command_Handle_Tasks(int argc, char **argv)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    strncpy(buffer, "Name\t\tState\tPrio\tStack\tTask#\n", sizeof(buffer));
    vTaskList(buffer + strlen(buffer));


    ESP_LOGI(TAG, "%s", buffer);
    return 0;
}

int Command_Handle_Restart(int argc, char **argv)
{
    esp_restart();
    return 0;
}