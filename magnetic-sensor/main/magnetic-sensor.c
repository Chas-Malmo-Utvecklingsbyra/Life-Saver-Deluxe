#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include <mdns.h>

#include "internet/internet.h"
#include "tcp/tcp_client.h"

#define SENSOR_GPIO_PORT GPIO_NUM_2
#define BYTES_TO_WORD(x) (x/4)

static const char* TAG = "Door/Window Sensor";

void sensor_read_task(void* params)
{
	int prev_state = -1;
	while (true)
	{
		int state = gpio_get_level(SENSOR_GPIO_PORT);

		if (state == 0 && state != prev_state)
		{
			ESP_LOGI(TAG, "CLOSED!");
			prev_state = state;
		}
		else if (state == 1 && state != prev_state)
		{
			ESP_LOGI(TAG, "OPEN!");
			prev_state = state;
		}
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void sensor_send_task(void* params)
{
	TCP_Client client = {};
	while (true)
	{
		if (TCP_Client_Initialize(&client) == TCP_Client_Success)
		{
			break;
		}
		else
		{
			ESP_LOGI("TCP_CLIENT", "Failed to Initialize!");
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	mdns_init();
	mdns_hostname_set("sensor-temp");

	esp_ip4_addr_t result = {};
	while (true)
	{
		esp_err_t mdns_query_err = mdns_query_a("homehub", 1000, &result);
		if (mdns_query_err != ESP_OK)
		{
			ESP_LOGI(TAG, "could not find address for home hub!");
			ESP_LOGI(TAG, "mDNS lookup failed: %s", esp_err_to_name(mdns_query_err));
			vTaskDelay(pdMS_TO_TICKS(100));
			continue;
		}
		break;
	}

	char ip_str[16];
	snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&result));

	while (true)
	{
		if (!Internet_Is_Connected())
		{
			vTaskDelay(pdMS_TO_TICKS(100));
			continue;
		}

		if (TCP_Client_Connect(&client, ip_str, (uint16_t)6060) == TCP_Client_Success)
		{
			break;
		}
		else
		{
			ESP_LOGI("TCP_CLIENT", "Failed to connect!");
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	const char* msg = "Whats up Home Hub?";
	TCP_Client_Send(&client, msg, strlen(msg));

	TCP_Client_Dispose(&client);
 	
	vTaskDelete(NULL);
}

void app_main(void)
{
    Internet_Initialize("username", "password");

	gpio_config_t config = {
		.pin_bit_mask = (1ULL << SENSOR_GPIO_PORT),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	if (gpio_config(&config) != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to setup GPIO port.");
		return;
	}

	BaseType_t read_task_result = xTaskCreate(sensor_read_task, "ReadTask", BYTES_TO_WORD(4096), NULL, 10, NULL);
	if (read_task_result != pdPASS)
	{
		// Task failed to be created
		ESP_LOGE(TAG, "ReadTask failed to be created!");
		return;
	}

	BaseType_t send_task_result = xTaskCreate(sensor_send_task, "SendTask", BYTES_TO_WORD(4096), NULL, 10, NULL);
	if (send_task_result != pdPASS)
	{
		// Task failed to be created
		ESP_LOGE(TAG, "SendTask failed to be created!");
		return;
	}
}
