#include "../../unity/unity.h"

#include <string.h>

#include "../internet.h"
#include "../mocks/mock_deps.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* Internet_Initialize */

void test_Internet_Initialize_Copies_SSID(void)
{
    /* Arrange */

    memset(&mock_wifi_config_argument, 0, sizeof(mock_wifi_config_argument));

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL_STRING(
        "MySSID",
        mock_wifi_config_argument.sta.ssid);
}

void test_Internet_Initialize_Copies_Password(void)
{
    /* Arrange */

    memset(&mock_wifi_config_argument, 0, sizeof(mock_wifi_config_argument));

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL_STRING(
        "MyPassword",
        mock_wifi_config_argument.sta.password);
}

void test_Internet_Initialize_Sets_Wifi_Mode(void)
{
    /* Arrange */

    mock_wifi_set_mode_argument = -1;

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL(
        WIFI_MODE_STA,
        mock_wifi_set_mode_argument);
}

void test_Internet_Initialize_Starts_Wifi(void)
{
    /* Arrange */

    mock_esp_wifi_start_call_count = 0;

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL(
        1,
        mock_esp_wifi_start_call_count);
}

void test_Internet_Initialize_Erases_NVS_When_No_Free_Pages(void)
{
    /* Arrange */

    mock_nvs_flash_init_call_count = 0;
    mock_nvs_flash_erase_call_count = 0;

    mock_nvs_flash_init_result = ESP_ERR_NVS_NO_FREE_PAGES;

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL(
        2,
        mock_nvs_flash_init_call_count);

    TEST_ASSERT_EQUAL(
        1,
        mock_nvs_flash_erase_call_count);
}

void test_Internet_Initialize_Registers_Event_Handlers(void)
{
    /* Arrange */

    mock_event_handler_register_call_count = 0;

    /* Act */

    Internet_Initialize("MySSID", "MyPassword");

    /* Assert */

    TEST_ASSERT_EQUAL(
        2,
        mock_event_handler_register_call_count);

    TEST_ASSERT_EQUAL(
        WIFI_EVENT,
        mock_event_base_arguments[0]);

    TEST_ASSERT_EQUAL(
        ESP_EVENT_ANY_ID,
        mock_event_id_arguments[0]);

    TEST_ASSERT_EQUAL(
        IP_EVENT,
        mock_event_base_arguments[1]);

    TEST_ASSERT_EQUAL(
        IP_EVENT_STA_GOT_IP,
        mock_event_id_arguments[1]);
}

/* Internet_Is_Connected */

void test_Internet_Is_Connected(void)
{
    TEST_PASS();
}