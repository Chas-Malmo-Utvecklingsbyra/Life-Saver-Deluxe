#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <nvs_flash.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "internet/internet.h"

#define SENSOR_GPIO_PORT GPIO_NUM_2
#define BYTES_TO_WORD(x) (x/4)

static const char* TAG = "Door/Window Sensor";

void sensor_read_task(void* params)
{
	while (true)
	{
		int state = gpio_get_level(SENSOR_GPIO_PORT);

		if (state == 0)
			ESP_LOGI(TAG, "CLOSED!");
		else
			ESP_LOGI(TAG, "OPEN!");
		
		vTaskDelay(100);
	}
}

void app_main(void)
{
	nvs_flash_init();
	Internet_Initialize_Minimal();
	if (esp_now_init() != ESP_OK)
	{
		ESP_LOGI(TAG, "Failed to Initialize ESP-NOW");
		return;
	}

	gpio_config_t config = {
		.pin_bit_mask = (1ULL << SENSOR_GPIO_PORT),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&config);

	BaseType_t read_task_result = xTaskCreate(sensor_read_task, "ReadTask", BYTES_TO_WORD(4096), NULL, 10, NULL);
	if (read_task_result != pdPASS)
	{
		// Task failed to be created
		ESP_LOGE(TAG, "ReadTask failed to be created! %s");
	}

	uint8_t receiver_mac[] = {0x90, 0x70, 0x69, 0x0A, 0x20, 0xC0}; // one of our ESP32S3
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, receiver_mac, 6);
    peer.channel = 1;
    peer.encrypt = false;

	esp_now_add_peer(&peer);


	uint8_t primary;
	wifi_second_chan_t second;

	esp_wifi_get_channel(&primary, &second);

	ESP_LOGI("WIFI", "Current channel: %d", primary);

	const char *msg = "Hello from ESPC3";

    while (1) {
		ESP_LOGI(TAG, "Trying to send data");
        esp_err_t res = esp_now_send(receiver_mac, (uint8_t *)msg, strlen(msg));
		ESP_LOGI(TAG, "send result: %s", esp_err_to_name(res));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
