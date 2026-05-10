#include "internet.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_log.h>

#include <stdatomic.h>

#ifdef CONFIG_INTERNET_TEST_HTTP

#include <esp_http_client.h>

// This is used to test the WIFI to see if we get a response
static void http_test()
{
    esp_http_client_config_t config = {
        .url = "http://httpbin.org/get",
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI("HTTP", "Test Status = %d",
                 esp_http_client_get_status_code(client));
    }
    else
    {
        ESP_LOGI("HTTP", "Test Request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

static TaskHandle_t http_task_handler = NULL;
static void http_task(void *params)
{
    while (1)
    {
        http_test();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif

static atomic_bool g_internet_connected = false;
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            g_internet_connected = false;

            wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t*)event_data;
            ESP_LOGI("WIFI", "Disconnected. Reason: %d (%s)", event->reason, esp_err_to_name(event->reason));
            esp_wifi_scan_start(NULL, true);
            ESP_LOGI("WIFI", "Trying to reconnect to WIFI");
            esp_wifi_connect();
        }
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("WIFI", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        g_internet_connected = true;

        #ifdef CONFIG_INTERNET_TEST_HTTP

        if (http_task_handler == NULL)
            xTaskCreate(http_task, "HttpTask", 4096, NULL, 5, &http_task_handler);

        #endif
    }
}


void Internet_Initialize(const char *ssid, const char *password)
{
    /* TODO: Remove ESP_ERROR_CHECK and add real safety checks that dont abort */
    esp_err_t nvs_init_result = nvs_flash_init();
    if (nvs_init_result == ESP_ERR_NVS_NO_FREE_PAGES || nvs_init_result == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                    ESP_EVENT_ANY_ID,
                                                    &wifi_event_handler,
                                                    NULL,
                                                    NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {};
    strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool Internet_Is_Connected()
{
    return g_internet_connected;
}