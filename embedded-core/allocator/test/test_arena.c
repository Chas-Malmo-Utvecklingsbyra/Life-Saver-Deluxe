#include "../../unity/unity.h"

#include "../arena.h"

void setUp(void)
{
    Arena_Reset();
}

void tearDown(void)
{
}

/* Arena_Malloc */

void test_Arena_Malloc_Returns_Buffer_On_First_Allocation(void)
{
    /* Act */

    void *ptr = Arena_Malloc(10);

    /* Assert */

    TEST_ASSERT_NOT_NULL(ptr);
}

void test_Arena_Malloc_Returns_Next_Free_Buffer(void)
{
    /* Act */

    uint8_t *ptr1 = Arena_Malloc(10);
    uint8_t *ptr2 = Arena_Malloc(20);

    /* Assert */

    TEST_ASSERT_EQUAL_PTR(
        ptr1 + 10,
        ptr2);
}

void test_Arena_Malloc_Returns_NULL_When_Request_Is_Too_Large(void)
{
    /* Act */

    void *ptr = Arena_Malloc(ALLOCATOR_ARENA_SIZE + 1);

    /* Assert */

    TEST_ASSERT_NULL(ptr);
}

void test_Arena_Malloc_Returns_NULL_When_Out_Of_Memory(void)
{
    /* Arrange */

    void *ptr1 = Arena_Malloc(ALLOCATOR_ARENA_SIZE);

    /* Act */

    void *ptr2 = Arena_Malloc(1);

    /* Assert */

    TEST_ASSERT_NOT_NULL(ptr1);

    TEST_ASSERT_NULL(ptr2);
}

void test_Arena_Malloc_Can_Allocate_Entire_Arena(void)
{
    /* Act */

    void *ptr = Arena_Malloc(ALLOCATOR_ARENA_SIZE);

    /* Assert */

    TEST_ASSERT_NOT_NULL(ptr);
}

/* Arena_Reset */

void test_Arena_Reset_Clears_Allocator(void)
{
    /* Arrange */

    Arena_Malloc(100);

    /* Act */

    Arena_Reset();

    void *ptr = Arena_Malloc(100);

    /* Assert */

    TEST_ASSERT_NOT_NULL(ptr);
}