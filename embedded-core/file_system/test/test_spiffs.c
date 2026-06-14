#include "../../unity/unity.h"
#include "../mocks/esp_spiffs.h"
#include "../spiffs/spiffs.h"

void setUp(void)
{
    mock_esp_vfs_spiffs_register_result = true;
}

void tearDown(void) { /* nothing */ }


/* Spiffs_Initialize */

void test_Spiffs_Initialize_Returns_True_On_Success(void)
{
    mock_esp_vfs_spiffs_register_result = true;
    TEST_ASSERT_TRUE(Spiffs_Initialize());
}

void test_Spiffs_Initialize_Returns_False_When_Register_Fails(void)
{
    mock_esp_vfs_spiffs_register_result = false;
    TEST_ASSERT_FALSE(Spiffs_Initialize());
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_Spiffs_Initialize_Returns_True_On_Success);
    RUN_TEST(test_Spiffs_Initialize_Returns_False_When_Register_Fails);

    return UNITY_END();
}