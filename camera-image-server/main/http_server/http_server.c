#include <string.h>
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "json/cJSON.h"
#include "http_server.h"

static const char *TAG = "HTTP_SERVER";
static AddressConfig_t *ServerConfig = NULL;

/// @brief Handle POST requests to update the server configuration
/// @param req The HTTP request object
/// @return ESP_OK on success, ESP_FAIL on failure
static esp_err_t config_post_handler(httpd_req_t *req)
{
    char buf[512];
    int total_len = req->content_len;

    if (total_len >= (int)sizeof(buf))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Request body too large");
        return ESP_FAIL;
    }

    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON *obj_url = cJSON_GetObjectItem(root, "object_detection_server_url");
    if (cJSON_IsString(obj_url) && obj_url->valuestring)
    {
        strlcpy(ServerConfig->object_detection_server_url, obj_url->valuestring,
                sizeof(ServerConfig->object_detection_server_url));
        ESP_LOGI(TAG, "object_detection_server_url set to: %s", ServerConfig->object_detection_server_url);
    }

    cJSON *hub_url = cJSON_GetObjectItem(root, "homehub_server_url");
    if (cJSON_IsString(hub_url) && hub_url->valuestring)
    {
        strlcpy(ServerConfig->homehub_server_url, hub_url->valuestring,
                sizeof(ServerConfig->homehub_server_url));
        ESP_LOGI(TAG, "homehub_server_url set to: %s", ServerConfig->homehub_server_url);
    }

    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return ESP_OK;
}

/// @brief Handle GET requests to retrieve the current configuration
/// @param req The HTTP request object
/// @return ESP_OK on success, ESP_FAIL on failure
static esp_err_t config_get_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "object_detection_server_url", ServerConfig->object_detection_server_url);
    cJSON_AddStringToObject(root, "homehub_server_url", ServerConfig->homehub_server_url);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (!json_str)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON serialisation failed");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);
    free(json_str);
    return ESP_OK;
}

int http_server_setup(AddressConfig_t *config)
{
    ESP_LOGI(TAG, "Setting up HTTP server...");
    ServerConfig = config;

    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &server_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return -1;
    }

    httpd_uri_t config_post = {
        .uri      = "/config",
        .method   = HTTP_POST,
        .handler  = config_post_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server, &config_post);

    httpd_uri_t config_get = {
        .uri      = "/config",
        .method   = HTTP_GET,
        .handler  = config_get_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server, &config_get);

    ESP_LOGI(TAG, "HTTP server started on port %d", server_config.server_port);
    return 0;
}