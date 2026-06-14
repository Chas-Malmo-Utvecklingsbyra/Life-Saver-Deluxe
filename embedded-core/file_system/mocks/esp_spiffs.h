#ifndef MOCK_ESP_SPIFFS_H
#define MOCK_ESP_SPIFFS_H

#include <stdbool.h>

/* esp_err_t stub */
typedef int esp_err_t;
#define ESP_OK  0
#define ESP_FAIL -1

/* Mock control */
extern bool mock_esp_vfs_spiffs_register_result;

typedef struct {
    const char *base_path;
    const char *partition_label;
    int         max_files;
    bool        format_if_mount_failed;
} esp_vfs_spiffs_conf_t;

static inline esp_err_t esp_vfs_spiffs_register(const esp_vfs_spiffs_conf_t *conf)
{
    (void)conf;
    return mock_esp_vfs_spiffs_register_result ? ESP_OK : ESP_FAIL;
}

#endif /* MOCK_ESP_SPIFFS_H */