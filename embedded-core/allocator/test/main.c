#include "../../unity/unity.h"

extern void test_Arena_Malloc_Returns_Buffer_On_First_Allocation(void);
extern void test_Arena_Malloc_Returns_Next_Free_Buffer(void);
extern void test_Arena_Malloc_Returns_NULL_When_Request_Is_Too_Large(void);
extern void test_Arena_Malloc_Returns_NULL_When_Out_Of_Memory(void);
extern void test_Arena_Malloc_Can_Allocate_Entire_Arena(void);
extern void test_Arena_Reset_Clears_Allocator(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_Arena_Malloc_Returns_Buffer_On_First_Allocation);
    RUN_TEST(test_Arena_Malloc_Returns_Next_Free_Buffer);
    RUN_TEST(test_Arena_Malloc_Returns_NULL_When_Request_Is_Too_Large);
    RUN_TEST(test_Arena_Malloc_Returns_NULL_When_Out_Of_Memory);
    RUN_TEST(test_Arena_Malloc_Can_Allocate_Entire_Arena);
    RUN_TEST(test_Arena_Reset_Clears_Allocator);

    return UNITY_END();
}