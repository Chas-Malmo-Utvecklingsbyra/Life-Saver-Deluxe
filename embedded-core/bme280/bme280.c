#include "bme280.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "i2c/i2c.h"

static const char *TAG =        "BME280";

#define I2C_PORT                I2C_NUM_0
#define I2C_SDA                 GPIO_NUM_8
#define I2C_SCL                 GPIO_NUM_9

#define I2C_FREQ_HZ             100000

#define BME280_ADDR             0x77
#define BME280_REG_ID           0xD0
#define BME280_REG_CALIB        0x88
#define BME280_CTRL_HUM_REG     0xF2
#define BME280_CTRL_HUM_BYTE    0x01
#define BME280_CTRL_MEAS_REG    0xF4
#define BME280_CTRL_MEAS_BYTE   0x25
#define BME280_MEAS_DATA_REG    0xF7

#define I2C_TIMEOUT_MS  1000

typedef struct
{
    uint16_t T1;
	int16_t T2, T3;
	uint16_t P1;
	int16_t P2, P3, P4, P5, P6, P7, P8, P9;
	uint8_t H1;
	int16_t H2;
	uint8_t H3;
	int16_t H4, H5;
	int8_t H6;
} bme280_calib_t;

typedef struct
{
	int32_t adc_T;
	int32_t adc_P;
	int32_t adc_H;
} bme280_raw_t;

static esp_err_t bme280_register_read(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t i2c_master_init();
esp_err_t bme280_read_calibration();
static esp_err_t bme280_write_byte(uint8_t reg, uint8_t value);
esp_err_t bme280_start_measurement();
static int32_t bme280_compensate_T(int32_t adc_T);
static uint32_t bme280_compensate_P(int32_t adc_P);
static uint32_t bme280_compensate_H(int32_t adc_H);

static bme280_calib_t calib;
static i2c_master_bus_handle_t bus_handle = NULL;
static i2c_master_dev_handle_t dev_handle = NULL;
bme280_meas_t meas;

static esp_err_t bme280_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_TIMEOUT_MS);
}

static esp_err_t bme280_write_byte(uint8_t reg, uint8_t value)
{
    uint8_t write_buf[2] = {reg, value};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_TIMEOUT_MS);
}

esp_err_t i2c_master_init()
{
    bus_handle = i2c_get_bus();

    i2c_device_config_t dev_config =
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BME280_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    return ESP_OK;
}

esp_err_t bme280_read_calibration()
{
    uint8_t buf[26];

    esp_err_t err = bme280_register_read(BME280_REG_CALIB, buf, 26);
    if (err != ESP_OK) 
        return err;

    calib.T1 = (uint16_t)(buf[1] << 8) | buf[0];
	calib.T2 = (int16_t)(buf[3] << 8) | buf[2];
	calib.T3 = (int16_t)(buf[5] << 8) | buf[4];
	calib.P1 = (uint16_t)(buf[7] << 8) | buf[6];
	calib.P2 = (int16_t)(buf[9] << 8) | buf[8];
	calib.P2 = (int16_t)(buf[11] << 8) | buf[10];
	calib.P2 = (int16_t)(buf[13] << 8) | buf[12];
	calib.P2 = (int16_t)(buf[15] << 8) | buf[14];
	calib.P2 = (int16_t)(buf[17] << 8) | buf[16];
	calib.P2 = (int16_t)(buf[19] << 8) | buf[18];
	calib.P2 = (int16_t)(buf[21] << 8) | buf[20];
	calib.P2 = (int16_t)(buf[23] << 8) | buf[22];
	calib.H1 = buf[25];

    err = bme280_register_read(BME280_REG_CALIB, buf, 7);
    if (err != ESP_OK) 
        return err;

    calib.H2 = (int16_t)(buf[1] << 8) | buf[0];
	calib.H3 = buf[2];
	calib.H4 = ((int16_t)buf[3] << 4) | (buf[4] & 0x0F);
	calib.H5 = ((int16_t)buf[5] << 4) | (buf[4] >> 4);
	calib.H6 = (int8_t)buf[6];

    ESP_LOGI(TAG, "Calibration data read: T1=%u T2=%d T3=%d", calib.T1, calib.T2, calib.T3);

    return ESP_OK;
}

esp_err_t bme280_start_measurement()
{
    esp_err_t err = bme280_write_byte(BME280_CTRL_HUM_REG, BME280_CTRL_HUM_BYTE);
    if (err != ESP_OK) 
        return err;
        
    err = bme280_write_byte(BME280_CTRL_MEAS_REG, BME280_CTRL_MEAS_BYTE);
    if (err != ESP_OK) 
        return err;

    vTaskDelay(pdMS_TO_TICKS(10));
    
    return ESP_OK;
}

static esp_err_t bme280_read_raw_meas(bme280_raw_t *raw)
{
	uint8_t buf[8];
	esp_err_t err = bme280_register_read(BME280_MEAS_DATA_REG, buf, 8);
	if(err != ESP_OK)
        return err;

	// for (int i = 0; i < 8; i++)
	// {
	// 	ESP_LOGI(TAG, "buf[%d]: %d", i, buf[i]);
	// }
	
	raw->adc_P = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | (buf[2] >> 4);
	raw->adc_T = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | (buf[5] >> 4);
	raw->adc_H = ((int32_t)buf[6] << 8) | buf[7];
	
	return ESP_OK;
}

static int32_t t_fine;
static int32_t bme280_compensate_T(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)calib.T1<<1))) * ((int32_t)calib.T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)calib.T1)) * ((adc_T>>4) - ((int32_t)calib.T1)))>>12)*((int32_t)calib.T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine*5 +128) >> 8;
	return T;
}

static uint32_t bme280_compensate_P(int32_t adc_P)
{
	int64_t var1, var2, P;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1*var1*(int64_t)calib.P6;
	var2 = var2 + ((var1*(int64_t)calib.P5)<<17);
	var2 = var2 + (((int64_t)calib.P4)<<35);
	var1 = ((var1*var1*(int64_t)calib.P3)>>8) + ((var1*(int64_t)calib.P2) << 12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)calib.P1)>>33;
	if(var1 == 0) return 0;
	P = 1048576-adc_P;
	P = (((P<<31)-var2)*3125)/var1;
	var1 = (((int64_t)calib.P9)*(P>>13)*(P>>13)) >> 25;
	var2 = (((int64_t)calib.P8)*P) >> 19;
	P = ((P + var1 + var2) >> 8) + (((int64_t)calib.P7) << 4);
	return (uint32_t) P;
}

static uint32_t bme280_compensate_H(int32_t adc_H)
{
	int32_t res;
	
	res = (t_fine - ((int32_t)76800));
	res = (((((adc_H << 14) - (((int32_t)calib.H4) << 20) - (((int32_t)calib.H5) * res)) + ((int32_t)16384)) >> 15) * (((((((res*((int32_t)calib.H6)) >> 10) * (((res*((int32_t)calib.H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib.H2) + 8192) >> 14));
	res = (res - (((((res >> 15) * (res >> 15)) >> 7) * ((int32_t)calib.H1)) >> 4));
	res = (res < 0 ? 0 : res);
	res = (res > 419430400 ? 419430400 : res);
	return (uint32_t) (res>>12);
}

esp_err_t bme280_read_meas(bme280_meas_t *meas)
{
    bme280_raw_t raw;

    esp_err_t err = bme280_read_raw_meas(&raw);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Did not receive measurements");
        return err;
    }

    meas->T = bme280_compensate_T(raw.adc_T);
    meas->P = bme280_compensate_P(raw.adc_P);
    meas->H = bme280_compensate_H(raw.adc_H);

    return ESP_OK;
}

esp_err_t bme280_work()
{
    
    i2c_master_init();
    ESP_LOGI(TAG, "I2C initialized successfully");

    uint8_t data;
    esp_err_t err = (bme280_register_read(BME280_REG_ID, &data, 1));
    if (err == ESP_OK)
    {
        if (data == 0x60)
        {
            ESP_LOGI(TAG, "Chip ID: 0x%02x (BME280, OK!)", data);
        }
        else
        {
            ESP_LOGW(TAG, "Chip ID: 0x%02x (Unexpected value)", data);
            return err;
        }
    }
    else
    {
        ESP_LOGW(TAG, "Chip ID read failed, no BME280 detected: %s", esp_err_to_name(err));
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
        return err;
    }

    err = bme280_read_calibration();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Calibration failed");
        return err;
    }

    while (1)
    {
        err = bme280_start_measurement();
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not start measurements");
            return err;
        }


        err = bme280_read_meas(&meas);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Did not receive measurements");
        }
        else
        {
            ESP_LOGI(TAG, "T: %u.%u DegC, P: %u Pa, H: %u %RH", meas.T / 100, meas.T % 100, meas.P / 256, meas.H / 1024);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    ESP_LOGI(TAG, "I2C de-initialized successfully");

    return ESP_OK;
}