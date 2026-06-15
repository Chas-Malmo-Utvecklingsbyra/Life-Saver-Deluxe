#include "../../unity/unity.h"

/* Test functions */

/* File_System_Initialize */
extern void test_Initialize_Spiffs_Fails_Returns_Error_Spiffs(void);
extern void test_Initialize_Spiffs_Success(void);
extern void test_Initialize_Returns_Already_Initialized_On_Second_Call(void);

/* File_System_Get_Type */
extern void test_Get_Type_Returns_Spiffs_After_Init(void);

/* File_System_Write_File */
extern void test_Write_File_Success(void);
extern void test_Write_File_Null_Text_Creates_File(void);
extern void test_Write_File_Fopen_Fails_Returns_Error_File(void);
extern void test_Write_File_Fprintf_Fails_Returns_Error_File_Write(void);
extern void test_Write_File_Long_Text_Success(void);
extern void test_Write_File_Multiple_Times(void);

/* File_System_Read_File */
extern void test_Read_File_Success_Returns_Correct_Data(void);
extern void test_Read_File_Fopen_Fails_Returns_Error_File(void);
extern void test_Read_File_Fgets_Fails_Returns_Error_File_Read(void);
extern void test_Read_File_Buffer_Truncated_To_Length(void);
extern void test_Read_File_Multiple_Times(void);

/* File_System_File_Exists */
extern void test_File_Exists_Returns_True_When_File_Found(void);
extern void test_File_Exists_Returns_False_When_File_Not_Found(void);
extern void test_File_Exists_Multiple_Times(void);

int main(void)
{
    UNITY_BEGIN();

    /* File_System_Initialize
       SPIFFS failure must be tested before successful initialization,
       since the initialized flag is static. */
    RUN_TEST(test_Initialize_Spiffs_Fails_Returns_Error_Spiffs);
    RUN_TEST(test_Initialize_Spiffs_Success);
    RUN_TEST(test_Initialize_Returns_Already_Initialized_On_Second_Call);

    /* File_System_Get_Type */
    RUN_TEST(test_Get_Type_Returns_Spiffs_After_Init);

    /* File_System_Write_File */
    RUN_TEST(test_Write_File_Success);
    RUN_TEST(test_Write_File_Null_Text_Creates_File);
    RUN_TEST(test_Write_File_Fopen_Fails_Returns_Error_File);
    RUN_TEST(test_Write_File_Fprintf_Fails_Returns_Error_File_Write);
    RUN_TEST(test_Write_File_Long_Text_Success);
    RUN_TEST(test_Write_File_Multiple_Times);

    /* File_System_Read_File */
    RUN_TEST(test_Read_File_Success_Returns_Correct_Data);
    RUN_TEST(test_Read_File_Fopen_Fails_Returns_Error_File);
    RUN_TEST(test_Read_File_Fgets_Fails_Returns_Error_File_Read);
    RUN_TEST(test_Read_File_Buffer_Truncated_To_Length);
    RUN_TEST(test_Read_File_Multiple_Times);

    /* File_System_File_Exists */
    RUN_TEST(test_File_Exists_Returns_True_When_File_Found);
    RUN_TEST(test_File_Exists_Returns_False_When_File_Not_Found);
    RUN_TEST(test_File_Exists_Multiple_Times);

    return UNITY_END();
}