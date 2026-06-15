#include "../../unity/unity.h"

#include <string.h>

#include "../random.h"
#include "../mocks/mock_deps.h"

void setUp(void)
{
    memset(mock_random_bytes, 0, sizeof(mock_random_bytes));
}

void tearDown(void)
{
}

/* Random_Generate_UUID_v4 */

void test_Random_Generate_UUID_v4_Returns_UUID_With_Correct_Length(void)
{
    /* Arrange */

    char uuid[RANDOM_MAX_UUID_V4_LENGTH];

    /* Act */

    Random_Generate_UUID_v4(uuid);

    /* Assert */

    TEST_ASSERT_EQUAL(
        36,
        strlen(uuid));
}

void test_Random_Generate_UUID_v4_Sets_Version_To_4(void)
{
    /* Arrange */

    char uuid[RANDOM_MAX_UUID_V4_LENGTH];

    memset(mock_random_bytes, 0, sizeof(mock_random_bytes));

    /* Act */

    Random_Generate_UUID_v4(uuid);

    /* Assert */

    TEST_ASSERT_EQUAL(
        '4',
        uuid[14]);
}

void test_Random_Generate_UUID_v4_Sets_RFC4122_Variant(void)
{
    /* Arrange */

    char uuid[RANDOM_MAX_UUID_V4_LENGTH];

    memset(mock_random_bytes, 0, sizeof(mock_random_bytes));

    /* Act */

    Random_Generate_UUID_v4(uuid);

    /* Assert */

    TEST_ASSERT_EQUAL(
        '8',
        uuid[19]);
}

void test_Random_Generate_UUID_v4_Formats_UUID_Correctly(void)
{
    /* Arrange */

    char uuid[RANDOM_MAX_UUID_V4_LENGTH];

    /* Act */

    Random_Generate_UUID_v4(uuid);

    /* Assert */

    TEST_ASSERT_EQUAL('-', uuid[8]);
    TEST_ASSERT_EQUAL('-', uuid[13]);
    TEST_ASSERT_EQUAL('-', uuid[18]);
    TEST_ASSERT_EQUAL('-', uuid[23]);
}

void test_Random_Generate_UUID_v4_Uses_Random_Data(void)
{
    /* Arrange */

    char uuid[RANDOM_MAX_UUID_V4_LENGTH];

    mock_random_bytes[0] = 0x12;
    mock_random_bytes[1] = 0x34;
    mock_random_bytes[2] = 0x56;
    mock_random_bytes[3] = 0x78;

    /* Act */

    Random_Generate_UUID_v4(uuid);

    /* Assert */

    TEST_ASSERT_EQUAL_STRING_LEN(
        "12345678",
        uuid,
        8);
}