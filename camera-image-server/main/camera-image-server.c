#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "wifi/connect_wifi.h"
#include "camera/camera.h"
#include "http_client/http_client.h"
#include "http_server/http_server.h"

#define BOARD_ESP32CAM_AITHINKER
#include "camera_pins.h"

#define IMAGE_SETTINGS 1
#define CONFIG_XCLK_FREQ 20000000 // 20000000

static const char *TAG = "camera-image-server";
static AddressConfig_t cameraConfig_g = {
    .object_detection_server_url = "http://192.168.1.5:8080",
    .homehub_server_url = "http://"
};



/// @brief Initialize the camera
/// @param
/// @return ESP_OK on success, error code otherwise
static esp_err_t init_camera(void)
{
    camera_config_t cameraJPEGConfig = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,
        .jpeg_quality = 10,
        .fb_count = 1,
        //.fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY};

    esp_err_t err = esp_camera_init(&cameraJPEGConfig);
    if (err != ESP_OK)
        return err;

    err = esp_camera_set_psram_mode(true);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set PSRAM mode: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}

/// @brief Set camera image settings
/// @return true on success, false on failure
bool set_camera_image_settings()
{
    sensor_t *sensor = esp_camera_sensor_get();

    if (!sensor)
        return false;

    // If you enable gain_ctrl or exposure_ctrl it will prevent a lot of the other settings having any effect
    // more info on settings here: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
    sensor->set_gain_ctrl(sensor, 1);            // auto gain off (1 or 0)
    sensor->set_exposure_ctrl(sensor, 1);        // auto exposure off (1 or 0)
    //sensor->set_agc_gain(sensor, 0);             // set gain manually (0 - 31)
    //sensor->set_aec_value(sensor, 0);            // set exposure manually  (0-1200)
    sensor->set_vflip(sensor, 0);                // Invert image (0 or 1)
    sensor->set_quality(sensor, 10);             // (0 - 63)
    sensor->set_gainceiling(sensor, GAINCEILING_32X); // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
    sensor->set_brightness(sensor, 0);                // (-2 to 2) - set brightness
    sensor->set_lenc(sensor, 1);                      // lens correction? (1 or 0)
    sensor->set_saturation(sensor, 0);                // (-2 to 2)
    sensor->set_contrast(sensor, 0);                  // (-2 to 2)
    sensor->set_sharpness(sensor, 0);                 // (-2 to 2)
    sensor->set_hmirror(sensor, 0);                   // (0 or 1) flip horizontally
    sensor->set_colorbar(sensor, 0);                  // (0 or 1) - show a testcard
    sensor->set_special_effect(sensor, 0);            // (0 to 6?) apply special effect
    sensor->set_bpc(sensor, 0);                       // black pixel correction
    sensor->set_wpc(sensor, 0);                       // white pixel correction

    // capture a frame to ensure settings apply (not sure if this is really needed)
    camera_fb_t *frame_buffer = esp_camera_fb_get(); // capture frame from camera
    esp_camera_fb_return(frame_buffer);              // return frame so memory can be released

    return true;
}

/// @brief Capture the current frame and POST it to the configured server URL
void send_image_post(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGI(TAG, "Failed to capture frame for POST");
        return;
    }
    
    int status_code = http_client_post(cameraConfig_g.object_detection_server_url, "/image", HTTP_CONTENT_TYPE_JPEG, (const char *)fb->buf, fb->len, NULL, 0);
    if (status_code < 0)
        ESP_LOGI(TAG, "Failed to send image POST request");
    else
        ESP_LOGI(TAG, "Image POST request sent successfully with status code: %d", status_code);

    esp_camera_fb_return(fb);
}

void camera_loop_task(void *parameters)
{

    
    while (true)
    {
        if(!capture_still())
        {
            ESP_LOGE(TAG, "Failed to capture image");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        
        uint16_t changes = motion_detect();

        if ((changes >= imageThresholdL) && (changes <= imageThresholdH))
        {
            update_frame();
            if(tCounter >= tCounterTrigger)
            {
                tCounter = 0;
                latestChanges = changes;
                ESP_LOGI(TAG, "Motion detected with %u changed blocks", changes);
                send_image_post();
            }
            else
            {
                ESP_LOGI(TAG, "Motion suspected with %u changed blocks, waiting for confirmation (%u/%u)", changes, tCounter, tCounterTrigger);
            }
        }
        else
        {
            update_frame();
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay for 1 second before capturing next image
    }
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    ESP_LOGI(TAG, "NVS initialized successfully\n");
    
#ifdef CONFIG_WIFI_SSID
    connect_wifi();
    #else
    ESP_LOGI(TAG, "No Wi-Fi credentials provided, skipping Wi-Fi connection\n");
#endif
    

    if (wifi_connect_status)
        ESP_LOGI(TAG, "Connected to Wi-Fi successfully\n");
    else
        ESP_LOGI(TAG, "Failed to connected with Wi-Fi, check your network Credentials\n");
    
    esp_err_t err = init_camera();
    if (err != ESP_OK)
    {
        printf("err: %s\n", esp_err_to_name(err));
        return;
    }

    if (!set_camera_image_settings())
    {
        ESP_LOGE(TAG, "Failed to apply camera image settings");
        return;
    }
    
    if (!camera_motion_init())
    {
        ESP_LOGE(TAG, "Failed to allocate motion detection buffer");
        return;
    }
    
    xTaskCreate(camera_loop_task, "camera_loop_task", 16384, NULL, 5, NULL);
    ESP_LOGI(TAG, "Camera loop task started\n");

    http_server_setup(&cameraConfig_g);
    
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Keep main task alive
    }
}