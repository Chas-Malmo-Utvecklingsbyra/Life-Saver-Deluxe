#include "uart.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define UART_PORT       UART_NUM_1
#define UART_TX_PIN     17
#define UART_RX_PIN     16
#define UART_BAUD       115200
#define BUF_SIZE        1024

static const char *TAG = "UART";

void uart_init(void)
{
    const uart_config_t uart_config = 
    {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_PORT,
        UART_TX_PIN,
        UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT,
        BUF_SIZE * 2,
        0,
        0,
        NULL,
        0));

    ESP_LOGI(TAG, "UART%d initialized at %d baud", UART_PORT, UART_BAUD);
}

void uart_rx_task(void *arg)
{
    uint8_t *buf = (uint8_t *)malloc(BUF_SIZE);
    if (buf == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UART RX task started");

    while (1)
    {
        int len = uart_read_bytes(UART_PORT, buf,
                                    BUF_SIZE - 1,
                                    pdMS_TO_TICKS(20));

        if (len > 0)
        {
            buf[len] = '\0';
            ESP_LOGI(TAG, "RX (%d): %s", len, buf);
            // Echo
            // uart_write_bytes(UART_PORT, (char *)buf, len);
        }
    }
    free(buf);
}