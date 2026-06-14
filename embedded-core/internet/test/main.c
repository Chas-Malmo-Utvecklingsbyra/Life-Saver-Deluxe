#include "../../unity/unity.h"

/* Test functions */
extern void test_Internet_Initialize_Copies_SSID(void);
extern void test_Internet_Initialize_Copies_Password(void);
extern void test_Internet_Initialize_Sets_Wifi_Mode(void);
extern void test_Internet_Initialize_Starts_Wifi(void);
extern void test_Internet_Initialize_Erases_NVS_When_No_Free_Pages(void);
extern void test_Internet_Initialize_Registers_Event_Handlers(void);
extern void test_Internet_Is_Connected(void);

int main(void)
{

    UNITY_BEGIN();

    RUN_TEST(test_Internet_Initialize_Copies_SSID);
    RUN_TEST(test_Internet_Initialize_Copies_Password);
    RUN_TEST(test_Internet_Initialize_Sets_Wifi_Mode);
    RUN_TEST(test_Internet_Initialize_Starts_Wifi);
    RUN_TEST(test_Internet_Initialize_Erases_NVS_When_No_Free_Pages);
    RUN_TEST(test_Internet_Initialize_Registers_Event_Handlers);
    RUN_TEST(test_Internet_Is_Connected);

    return UNITY_END();
}