#ifndef MOCK_ESP_EVENT_H
#define MOCK_ESP_EVENT_H

#include <stdint.h>
#include "esp_wifi.h"

typedef int esp_event_base_t;

#define ESP_EVENT_ANY_ID (-1)

#define IP_EVENT 2
#define IP_EVENT_STA_GOT_IP 200

typedef struct
{
    struct
    {
        uint32_t ip;
    } ip_info;
} ip_event_got_ip_t;

/* Function prototypes */

esp_err_t esp_event_loop_create_default(void);

esp_err_t esp_event_handler_instance_register(
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_handler,
    void *event_handler_arg,
    void *instance);

#define IPSTR "%u.%u.%u.%u"

#define IP2STR(ip) 0, 0, 0, 0

#endif