#include <stdio.h>
#include <stdatomic.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include <mdns.h>

#include "internet/internet.h"
#include "tcp/tcp_client.h"
#include "tcp/packet/packet.h"
#include "file_system/file_system.h"
#include "random/random.h"
#include "json/cJSON.h"
#include "allocator/arena.h"

#define SENSOR_GPIO_PORT GPIO_NUM_2
#define BYTES_TO_WORD(x) (x/4)

#define CFG_NAME "uuid.cfg"

static const char* TAG = "Door/Window Sensor";

static TCP_Client client = {};
static char guid[RANDOM_MAX_UUID_V4_LENGTH];
static atomic_bool has_guid = false;

static void send_data_to_home_hub(const char* data)
{
	if (has_guid && Internet_Is_Connected())
	{
		char buffer[256];
		snprintf(buffer, 256, "%s|%s", guid, data);

		char* packet = Packet_Build(Packet_Job_Data, buffer);
		if (TCP_Client_Send(&client, packet, strlen(packet)) != TCP_Client_Success)
		{
			ESP_LOGW(TAG, "Could not send data packet to server.");
		}
		free(packet);
	}
}

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

			send_data_to_home_hub("false");
		}
		else if (state == 1 && state != prev_state)
		{
			ESP_LOGI(TAG, "OPEN!");
			prev_state = state;

			send_data_to_home_hub("true");
		}
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void sensor_send_task(void* params)
{
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

	if (!has_guid)
	{
		char* json = Packet_Build(Packet_Job_Initialize, "magnetic");
		if (TCP_Client_Send(&client, json, strlen(json)) != TCP_Client_Success)
		{
			ESP_LOGI(TAG, "Could not send Initialization packet to server.");
		}
		ESP_LOGI(TAG, "Successfully sent [%s]", json);
		free(json);
	}
	else
	{
		ESP_LOGI(TAG, "Not sending Packet_Job_Initialize, because I already have a UUID");
	}

	client.has_initialized = true;
	vTaskDelete(NULL);
}

void read_tcp_task(void* params)
{
	static bool do_once = false;

	while(1)
	{
		if (!client.has_initialized)
		{
			vTaskDelay(pdMS_TO_TICKS(100));
			continue;
		}

		if (!do_once)
		{
			int flags = fcntl(client.socket, F_GETFL, 0);
    		fcntl(client.socket, F_SETFL, flags | O_NONBLOCK);
			do_once = true;
		}

		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));

		int total_bytes = 0;

		while (true)
		{
			int bytes = recv(client.socket, &buffer[total_bytes], sizeof(buffer) - total_bytes, 0);

			if (bytes > 0)
			{
				total_bytes += bytes;
			}

			if (bytes == 0)
			{
				break;
			}

			if (bytes < 0)
			{
				break;
			}

			ESP_LOGI(TAG, "Looppoop");
		}

		if (total_bytes != 0)
		{
			cJSON* root = cJSON_Parse(buffer);
			if (root == NULL) // Illegal request
			{
				ESP_LOGE(TAG, "Illegal TCP was received continuing...");
				vTaskDelay(100);
				continue;
			}

			cJSON* job = cJSON_GetObjectItem(root, "job");
			if (job == NULL)
			{
				Arena_Reset();
				ESP_LOGE(TAG, "Could not find JOB...");
				vTaskDelay(100);
				continue;
			}

			char* job_string = cJSON_GetStringValue(job);
			if (job_string == NULL)
			{
				Arena_Reset();
				ESP_LOGE(TAG, "Job is not a string...");
				vTaskDelay(100);
				continue;
			}

			switch (Packet_Job_From_String(job_string))
			{
				case Packet_Job_Initialize:
				{
					// Packet_Job_Initialize acts like a handshake, which we will get a GUID
					// from the server

					cJSON* data = cJSON_GetObjectItem(root, "data");
					if (data == NULL)
					{
						Arena_Reset();
						ESP_LOGE(TAG, "Could not find data even when it is EXPECTED...");
						vTaskDelay(100);
						continue;
					}

					char* string_data = cJSON_GetStringValue(data);
					if (string_data == NULL)
					{
						Arena_Reset();
						ESP_LOGE(TAG, "Could not parse DATA to string value...");
						vTaskDelay(100);
						continue;
					}

					ESP_LOGI(TAG, "Received from Initialization: [%s]", string_data);
					snprintf(guid, RANDOM_MAX_UUID_V4_LENGTH, "%s", string_data);
					
					has_guid = true;
					if (File_System_Write_File(CFG_NAME, guid, "w") != File_System_Success)
					{
						ESP_LOGE(TAG, "Failed to write to file the UUID");
					}

					Arena_Reset();
					break;
				}
				default:
				{
					break;
				}
			}

		}

		vTaskDelay(100);
	}

	TCP_Client_Dispose(&client);
}

void app_main(void)
{
	if (File_System_Initialize(File_System_Type_Spiffs) != File_System_Success)
	{
		ESP_LOGE(TAG, "Failed to Initialize File System!");
		return;
	}
	//remove("/spiffs/uuid.cfg");

	if (File_System_File_Exists(CFG_NAME))
	{
		char uuid4_buffer[RANDOM_MAX_UUID_V4_LENGTH];
		if (File_System_Read_File(CFG_NAME, uuid4_buffer, RANDOM_MAX_UUID_V4_LENGTH) != File_System_Success)
		{
			ESP_LOGE(TAG, "Failed to read uuid file...");
			return; // it should never fail
		}

		ESP_LOGE(TAG, "UUID4 found is: [%s]", uuid4_buffer);
		snprintf(guid, RANDOM_MAX_UUID_V4_LENGTH, "%s", uuid4_buffer);
		has_guid = true;
	}
	else
	{
		ESP_LOGI(TAG, "Could not find %s file.. Waiting for Initialization from TCP", CFG_NAME);
	}
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

	BaseType_t tcp_read_task_result = xTaskCreate(read_tcp_task, "TcpReadTask", 4096, NULL, 10, NULL);
	if (tcp_read_task_result != pdPASS)
	{
		// Task failed to be created
		ESP_LOGE(TAG, "TcpReadTask failed to be created!");
		return;
	}
}
