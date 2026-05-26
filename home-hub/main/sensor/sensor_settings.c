#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sensor.h"
#include "sensor_settings.h"

#define TAG "SENSOR_NAMES"
#define NAMESPACE "sensor_names"
#define TAG_PLACE "SENSOR_PLACEMENT"
#define NAMESPACE_PLACE "sensor_place"

static nvs_handle_t p;

static void guid_to_nvs_key(const char *guid, char key_out[16])
{
    uint32_t h = 0x811C9DC5u;
    for (const char *p = guid; *p; p++)
    {
        h ^= (uint8_t)*p;
        h *= 0x01000193u;
    }
    snprintf(key_out, 16, "%08" PRIx32, h);
}

void sensor_names_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "NVS partition truncated or version mismatch - erasing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    nvs_handle_t h;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s", NAMESPACE, esp_err_to_name(err));
        return;
    }
    nvs_close(h);
    ESP_LOGI(TAG, "NVS namespace '%s' ready", NAMESPACE);
}

bool sensor_names_get(const char *guid, char *out, size_t out_size)
{
    if (!guid || !out || out_size == 0)
    {
        return false;
    }

    char key[16];
    guid_to_nvs_key(guid, key);

    nvs_handle_t h;
    if (nvs_open(NAMESPACE, NVS_READONLY, &h) != ESP_OK)
    {
        return false;
    }

    size_t required = out_size;
    esp_err_t err = nvs_get_str(h, key, out, &required);
    nvs_close(h);

    if (err == ESP_OK)
    {
        ESP_LOGD(TAG, "get '%s' -> '%s'", guid, out);
        return true;
    }

    return false;
}

void sensor_names_set(const char *guid, const char *name)
{
    if (!guid || !name)
    {
        return;
    }

    char key[16];
    guid_to_nvs_key(guid, key);

    nvs_handle_t h;
    if (nvs_open(NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
    {
        ESP_LOGE(TAG, "Cannot open NVS for write");
        return;
    }

    esp_err_t err = nvs_set_str(h, key, name);
    if (err == ESP_OK)
    {
        nvs_commit(h);
        ESP_LOGI(TAG, "set '%s' -> '%s'", guid, name);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to set NVS to string: %s", esp_err_to_name(err));
    }
    
    nvs_close(h);
}

void sensor_placement_init(void)
{
    esp_err_t err = nvs_open(NAMESPACE_PLACE, NVS_READWRITE, &p);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_PLACE, "Failed to open NVS namespace (%s)", esp_err_to_name(err));
    }
}

bool sensor_placement_set(const char *guid, SensorPlacement placement)
{
    if (guid == NULL || guid[0] == '\0')
        return false;

    char key[16];
    guid_to_nvs_key(guid, key);

    esp_err_t err;

    err = nvs_set_u8(p, key, (uint8_t)placement);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_PLACE, "Failed to save placement for %s (%s)", guid, esp_err_to_name(err));
        return false;
    }

    err = nvs_commit(p);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_PLACE, "Failed to commit placement (%s)", esp_err_to_name(err));
        return false;
    }

    return true;
}

bool sensor_placement_get(const char *guid, SensorPlacement *placement)
{
    if (guid == NULL || placement == NULL)
        return false;

    char key[16];
    guid_to_nvs_key(guid, key);

    uint8_t stored_value = 0;

    esp_err_t err = nvs_get_u8(p, key, &stored_value);
    if (err != ESP_OK)
    {
        return false;
    }

    *placement = (SensorPlacement)stored_value;

    return true;
}