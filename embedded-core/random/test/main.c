#include "../../unity/unity.h"

extern void test_Random_Generate_UUID_v4_Returns_UUID_With_Correct_Length(void);
extern void test_Random_Generate_UUID_v4_Sets_Version_To_4(void);
extern void test_Random_Generate_UUID_v4_Sets_RFC4122_Variant(void);
extern void test_Random_Generate_UUID_v4_Formats_UUID_Correctly(void);
extern void test_Random_Generate_UUID_v4_Uses_Random_Data(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_Random_Generate_UUID_v4_Returns_UUID_With_Correct_Length);
    RUN_TEST(test_Random_Generate_UUID_v4_Sets_Version_To_4);
    RUN_TEST(test_Random_Generate_UUID_v4_Sets_RFC4122_Variant);
    RUN_TEST(test_Random_Generate_UUID_v4_Formats_UUID_Correctly);
    RUN_TEST(test_Random_Generate_UUID_v4_Uses_Random_Data);

    return UNITY_END();
}