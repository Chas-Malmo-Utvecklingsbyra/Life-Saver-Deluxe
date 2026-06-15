#include "../../unity/unity.h"

extern void test_i2c_init_Returns_ESP_OK_When_Bus_Is_Created(void);
extern void test_i2c_init_Returns_Error_When_Bus_Creation_Fails(void);
extern void test_i2c_get_bus_Returns_NULL_Before_Initialization(void);
extern void test_i2c_get_bus_Returns_Bus_Handle_After_Initialization(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_i2c_init_Returns_ESP_OK_When_Bus_Is_Created);
    RUN_TEST(test_i2c_init_Returns_Error_When_Bus_Creation_Fails);
    RUN_TEST(test_i2c_get_bus_Returns_NULL_Before_Initialization);
    RUN_TEST(test_i2c_get_bus_Returns_Bus_Handle_After_Initialization);

    return UNITY_END();
}