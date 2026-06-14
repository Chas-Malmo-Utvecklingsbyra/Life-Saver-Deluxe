#include "../../unity/unity.h"
#include "../mocks/mock_deps.h"
#include "../file_system.h"

static void reset_mocks(void)
{
    mock_spiffs_init_result = true;
    mock_fopen_fail = false;
    mock_fprintf_fail = false;
    mock_fgets_fail = false;
    memset(mock_file_content, 0, sizeof(mock_file_content));
}

void setUp(void) { reset_mocks(); }
void tearDown(void) { /* nothing */ }

/* File_System_Initialize
   NOTE: the static initialized flag is NOT reset between
   tests — therefore the order in main() is significant */

void test_Initialize_Spiffs_Fails_Returns_Error_Spiffs(void)
{
    /* Must run before a successful initialization,
       otherwise Already_Initialized will be returned */
    mock_spiffs_init_result = false;
    File_System_Error err = File_System_Initialize(File_System_Type_Spiffs);
    TEST_ASSERT_EQUAL(File_System_Error_Spiffs, err);
}

void test_Initialize_Spiffs_Success(void)
{
    mock_spiffs_init_result = true;
    File_System_Error err = File_System_Initialize(File_System_Type_Spiffs);
    TEST_ASSERT_EQUAL(File_System_Success, err);
}

void test_Initialize_Returns_Already_Initialized_On_Second_Call(void)
{
    File_System_Error err = File_System_Initialize(File_System_Type_Spiffs);
    TEST_ASSERT_EQUAL(File_System_Error_Already_Initialized, err);
}

/* File_System_Get_Type */

void test_Get_Type_Returns_Spiffs_After_Init(void)
{
    TEST_ASSERT_EQUAL(File_System_Type_Spiffs, File_System_Get_Type());
}

/* File_System_Write_File */

void test_Write_File_Success(void)
{
    File_System_Error err = File_System_Write_File("test.txt", "hello", "w");
    TEST_ASSERT_EQUAL(File_System_Success, err);
}

void test_Write_File_Null_Text_Creates_File(void)
{
    /* NULL text = create file without content */
    File_System_Error err = File_System_Write_File("empty.txt", NULL, "w");
    TEST_ASSERT_EQUAL(File_System_Success, err);
}

void test_Write_File_Fopen_Fails_Returns_Error_File(void)
{
    mock_fopen_fail = true;
    File_System_Error err = File_System_Write_File("test.txt", "data", "w");
    TEST_ASSERT_EQUAL(File_System_Error_File, err);
}

void test_Write_File_Fprintf_Fails_Returns_Error_File_Write(void)
{
    mock_fprintf_fail = true;
    File_System_Error err = File_System_Write_File("test.txt", "data", "w");
    TEST_ASSERT_EQUAL(File_System_Error_File_Write, err);
}

/* File_System_Read_File */

void test_Read_File_Success_Returns_Correct_Data(void)
{
    strncpy(mock_file_content, "stored data", sizeof(mock_file_content));
    char buf[64] = {0};
    File_System_Error err = File_System_Read_File("test.txt", buf, sizeof(buf));
    TEST_ASSERT_EQUAL(File_System_Success, err);
    TEST_ASSERT_EQUAL_STRING("stored data", buf);
}

void test_Read_File_Fopen_Fails_Returns_Error_File(void)
{
    mock_fopen_fail = true;
    char buf[64] = {0};
    File_System_Error err = File_System_Read_File("missing.txt", buf, sizeof(buf));
    TEST_ASSERT_EQUAL(File_System_Error_File, err);
}

void test_Read_File_Fgets_Fails_Returns_Error_File_Read(void)
{
    mock_fgets_fail = true;
    char buf[64] = {0};
    File_System_Error err = File_System_Read_File("test.txt", buf, sizeof(buf));
    TEST_ASSERT_EQUAL(File_System_Error_File_Read, err);
}

void test_Read_File_Buffer_Truncated_To_Length(void)
{
    strncpy(mock_file_content, "1234567890", sizeof(mock_file_content));
    char buf[5] = {0};
    File_System_Error err = File_System_Read_File("test.txt", buf, sizeof(buf));
    TEST_ASSERT_EQUAL(File_System_Success, err);
    TEST_ASSERT_EQUAL(4, strlen(buf)); /* max 4 characters + '\0' */
}

/* File_System_File_Exists */

void test_File_Exists_Returns_True_When_File_Found(void)
{
    mock_fopen_fail = false;
    TEST_ASSERT_TRUE(File_System_File_Exists("exists.txt"));
}

void test_File_Exists_Returns_False_When_File_Not_Found(void)
{
    mock_fopen_fail = true;
    TEST_ASSERT_FALSE(File_System_File_Exists("missing.txt"));
}

void test_Write_File_Long_Text_Success(void)
{
    char text[200];

    memset(text, 'A', sizeof(text) - 1);
    text[sizeof(text) - 1] = '\0';

    File_System_Error err =
        File_System_Write_File("long.txt",
                               text,
                               "w");

    TEST_ASSERT_EQUAL(File_System_Success, err);
}

void test_Write_File_Multiple_Times(void)
{
    for (int i = 0; i < 100; i++)
    {
        File_System_Error err =
            File_System_Write_File("test.txt",
                                   "data",
                                   "w");

        TEST_ASSERT_EQUAL(File_System_Success, err);
    }
}

void test_Read_File_Multiple_Times(void)
{
    strncpy(mock_file_content,
            "stored data",
            sizeof(mock_file_content));

    for (int i = 0; i < 100; i++)
    {
        char buf[64] = {0};

        File_System_Error err =
            File_System_Read_File("test.txt",
                                  buf,
                                  sizeof(buf));

        TEST_ASSERT_EQUAL(File_System_Success, err);
    }
}

void test_File_Exists_Multiple_Times(void)
{
    mock_fopen_fail = false;

    for (int i = 0; i < 100; i++)
    {
        TEST_ASSERT_TRUE(
            File_System_File_Exists("exists.txt"));
    }
}

/* Main */

int main(void)
{
    UNITY_BEGIN();

    /* File_System_Initialize — SPIFFS failure must be tested first */
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

    /* File_System_Read_File */
    RUN_TEST(test_Read_File_Success_Returns_Correct_Data);
    RUN_TEST(test_Read_File_Fopen_Fails_Returns_Error_File);
    RUN_TEST(test_Read_File_Fgets_Fails_Returns_Error_File_Read);
    RUN_TEST(test_Read_File_Buffer_Truncated_To_Length);

    /* File_System_File_Exists */
    RUN_TEST(test_File_Exists_Returns_True_When_File_Found);
    RUN_TEST(test_File_Exists_Returns_False_When_File_Not_Found);

    RUN_TEST(test_Write_File_Long_Text_Success);

    RUN_TEST(test_Write_File_Multiple_Times);

    RUN_TEST(test_Read_File_Multiple_Times);

    RUN_TEST(test_File_Exists_Multiple_Times);

    return UNITY_END();
}