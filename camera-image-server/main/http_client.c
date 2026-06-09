#include "http_client.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG = "HTTP_CLIENT";

/// @brief Convert HTTP content type enum to string
/// @param content_type The HTTP content type enum
/// @return The corresponding string representation of the content type
static const char* http_client_content_type_to_string(Http_Client_Content_Type_e content_type)
{
    switch (content_type)
    {
    case HTTP_CONTENT_TYPE_OCTET_STREAM:
        return "application/octet-stream";
    case HTTP_CONTENT_TYPE_JPEG:
        return "image/jpeg";
    case HTTP_CONTENT_TYPE_PLAIN_TEXT:
        return "text/plain";
    case HTTP_CONTENT_TYPE_JSON:
        return "application/json";
    default:
        return "application/octet-stream"; // Default to binary stream if unknown
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    static int output_len;

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;

    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;

    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_HEADERS_COMPLETE:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADERS_COMPLETE");
        break;

    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

        if (output_len == 0 && evt->user_data)
            memset(evt->user_data, 0, evt->data_len);

        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                int content_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                    output_buffer = (char *)calloc(content_len + 1, sizeof(char));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (content_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;


    case HTTP_EVENT_REDIRECT: // Not used
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        //esp_http_client_set_header(evt->client, "From", "user@example.com");
        //esp_http_client_set_header(evt->client, "Accept", "text/html");
        //esp_http_client_set_redirection(evt->client);
        break;

    default:
        break;
    }
    return ESP_OK;
}

esp_err_t http_client_post(const char *url, const char *path, Http_Client_Content_Type_e content_type, const char *post_data, const uint16_t post_data_len, char *response_buffer, uint16_t buffer_size)
{
    ESP_LOGI(TAG, "Starting HTTP POST request to URL: %s%s", url, path);
    
    esp_http_client_config_t config = {
        .url = url,
        .path = path,
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .user_data = response_buffer,
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    esp_err_t err = esp_http_client_set_header(client, "Content-Type", http_client_content_type_to_string(content_type));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set HTTP header: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    err = esp_http_client_set_post_field(client, post_data, post_data_len);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set POST data: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    int status_code = esp_http_client_get_status_code(client);
    int content_length = esp_http_client_get_content_length(client);
    ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d", status_code, content_length);

    // Ensure the response is null-terminated
    if (response_buffer && buffer_size > 0)
        response_buffer[buffer_size - 1] = '\0';

    esp_http_client_cleanup(client);
    return ESP_OK;
}