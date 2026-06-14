#include "arena.h"

#include <string.h>

#ifdef UNIT_TEST

#include "mocks/mock_deps.h"

#else

#include "esp_log.h"

#endif

static Allocator_Arena internal = {0};

void* Arena_Malloc(size_t size)
{
    if (internal.bytes_in_use + size > ALLOCATOR_ARENA_SIZE)
    {
        ESP_LOGE("Arena Allocator", "Attempted to malloc more bytes than available (%zu/%zu)", internal.bytes_in_use + size, ALLOCATOR_ARENA_SIZE);
        return NULL;
    }

    //ESP_LOGI("Arena Allocator", "Allocated byte (%zu/%zu)", internal.bytes_in_use + size, ALLOCATOR_ARENA_SIZE);


    size_t old_size = internal.bytes_in_use;
    internal.bytes_in_use += size;
    return &internal.buffer[old_size];
}

void Arena_Reset()
{
    memset(&internal, 0, sizeof(Allocator_Arena));
}