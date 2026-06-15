#include "../../unity/unity.h"
#include "../mocks/mock_deps.h"

typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1

typedef void * i2c_bus_handle_t;

void i2c_reset(void);
esp_err_t i2c_init(void);
i2c_bus_handle_t i2c_get_bus(void);

void setUp(void)
{
    i2c_reset();

    mock_i2c_new_master_bus_result = ESP_OK;
    mock_i2c_new_master_bus_call_count = 0;
}

void tearDown(void)
{
}

/* i2c_init */

void test_i2c_init_Returns_ESP_OK_When_Bus_Is_Created(void)
{
    /* Arrange */

    mock_i2c_new_master_bus_result = ESP_OK;

    /* Act */

    esp_err_t result = i2c_init();

    /* Assert */

    TEST_ASSERT_EQUAL(
        ESP_OK,
        result);
}

void test_i2c_init_Returns_Error_When_Bus_Creation_Fails(void)
{
    /* Arrange */

    mock_i2c_new_master_bus_result = ESP_FAIL;

    /* Act */

    esp_err_t result = i2c_init();

    /* Assert */

    TEST_ASSERT_EQUAL(
        ESP_FAIL,
        result);
}

void test_i2c_init_Initializes_Only_Once(void)
{
    /* Arrange */

    mock_i2c_new_master_bus_call_count = 0;

    /* Act */

    i2c_init();
    i2c_init();

    /* Assert */

    TEST_ASSERT_EQUAL(
        1,
        mock_i2c_new_master_bus_call_count);
}

/* i2c_get_bus */

void test_i2c_get_bus_Returns_NULL_Before_Initialization(void)
{
    TEST_ASSERT_NULL(
        i2c_get_bus());
}

void test_i2c_get_bus_Returns_Bus_Handle_After_Initialization(void)
{
    /* Arrange */

    i2c_init();

    /* Assert */

    TEST_ASSERT_EQUAL_PTR(
        mock_bus_handle,
        i2c_get_bus());
}